#include "instructionSelector.h"

InstructionSelector::InstructionSelector(std::shared_ptr<IR> _ir):ir(_ir)
{
	P = std::make_shared<RISCVProgram>();
	visit(ir.get());
}

std::string InstructionSelector::getLabel(const std::string & label)
{
	auto ret = label + std::to_string(labelCnt[label]);
	labelCnt[label]++;
	return ret;
}

void InstructionSelector::visit(IR * ir)
{
	for (auto &f : ir->getBuiltInFunctions()) {
		auto name = f->getName();
		for (int i = 0; i < name.size(); i++)
			if (name[i] == '.') name[i] = '_';
		irFunc2RISCV[f.get()] = std::make_shared<RISCVFunction>(name);
	}

	auto functions = ir->getFunctions();
	for (auto &f : functions) {
		auto ff = std::make_shared<RISCVFunction>(f->getName());
		irFunc2RISCV[f.get()] = ff;
		P->appendFunction(ff);
	}
	for (auto &f : functions) f->accept(*this);
}

void InstructionSelector::visit(Function * f)
{
	currentFunction = irFunc2RISCV[f];
	auto blocks = f->getBlockList();
	for (auto &b : blocks) {
		auto bb = std::make_shared<RISCVBasicBlock>(currentFunction,
			getLabel(currentFunction->getName() + "_" + b->toString()));
		irBlock2RISCV[b.get()] = bb;
		currentFunction->appendBlock(bb);
	}
	currentFunction->setEntry(irBlock2RISCV[f->getEntry().get()]);
	currentFunction->setExit(irBlock2RISCV[f->getExit().get()]);

	functionEntryBlockInit(f, currentFunction->getEntry());
	for (auto &b : blocks) b->accept(*this);
}

void InstructionSelector::visit(BasicBlock * b)
{
	currentBlock = irBlock2RISCV[b];
	for (auto i = b->getFront(); i != nullptr; i = i->getNextInstr())
		i->accept(*this);
}

void InstructionSelector::visit(Quadruple * q)
{
	auto op = q->getOp();
	if (op == Quadruple::STORE) {
		auto addr = std::make_shared<BaseOffsetAddr>(toRegister(q->getDst()), 0);
		if (addr->getBase()->isGlobal()) {
			auto reg = std::make_shared<VirtualReg>(Operand::REG_VAL, "store_glb");
			currentBlock->append(std::make_shared<Store>(currentBlock, addr,
				toRegister(q->getSrc1()), Configuration::SIZE_OF_INT, reg));
		}
		else currentBlock->append(std::make_shared<Store>(currentBlock,addr, 
				toRegister(q->getSrc1()), Configuration::SIZE_OF_INT));
	}
	else if (op == Quadruple::LOAD) {
		auto addr = std::make_shared<BaseOffsetAddr>(toRegister(q->getSrc1()),0);
		currentBlock->append(std::make_shared<Load>(currentBlock,
			addr,
			std::static_pointer_cast<Register>(q->getDst()), Configuration::SIZE_OF_INT));
	}
	else if (op == Quadruple::MOVE) {
		currentBlock->append(std::make_shared<MoveAssembly>(currentBlock,
			std::static_pointer_cast<Register>(q->getDst()), toRegister(q->getSrc1())));
	}
	else if (op == Quadruple::NEG) {
		auto rs1 = toRegister(q->getSrc1());
		auto rd = std::static_pointer_cast<Register>(q->getDst());
		currentBlock->append(std::make_shared<R_type>(currentBlock,
			R_type::SUB, rd, (*P)["zero"], rs1));
	}
	else if (op == Quadruple::INV) {
		auto rs1 = toRegister(q->getSrc1());
		auto rd = std::static_pointer_cast<Register>(q->getDst());
		currentBlock->append(std::make_shared<I_type>(currentBlock,
			I_type::XORI, rd, rs1, std::make_shared<Immediate>(-1)));
	}
	else {
		if (isRtype(q->getSrc1(), op, q->getSrc2())) resolveRtype(q);
		else resolveItype(q);
	}
}

void InstructionSelector::visit(Branch * b)
{
	currentBlock->append(std::make_shared<B_type>(currentBlock,
		B_type::BNE, (*P)["zero"], toRegister(b->getCondition()),
		irBlock2RISCV[b->getTrueBlock().get()]));
	currentBlock->append(std::make_shared<JumpAssembly>(
		currentBlock, irBlock2RISCV[b->getFalseBlock().get()]));
}

void InstructionSelector::visit(Call * c)
{
	auto args = c->getArgs();
	if (c->getObjRef() != nullptr) args.insert(args.begin(), c->getObjRef());

	for (int i = 0; i < std::min(8, (int)args.size()); i++)
		currentBlock->append(std::make_shared<MoveAssembly>(currentBlock,
			(*P)["a" + std::to_string(i)], toRegister(args[i])));

	for (int i = 8; i < args.size(); i++) {
		auto pos = std::make_shared<StackLocation>(currentFunction, (*P)["sp"], (i - 8)*Configuration::SIZE_OF_INT);
		currentBlock->append(std::make_shared<Store>(currentBlock, pos, toRegister(args[i]), Configuration::SIZE_OF_INT));
	}

	currentFunction->setLowerBoundForStackSizeFromTop(Configuration::SIZE_OF_INT * (args.size() - 8));

	currentBlock->append(std::make_shared<CallAssembly>(currentBlock, irFunc2RISCV[c->getFunction().get()]));

	// return value
	if (c->getResult() != nullptr)
		currentBlock->append(std::make_shared<MoveAssembly>(currentBlock,
			std::static_pointer_cast<Register>(c->getResult()), (*P)["a0"]));
}

void InstructionSelector::visit(Malloc * m)
{
	currentBlock->append(std::make_shared<MoveAssembly>(currentBlock, (*P)["a0"], toRegister(m->getSize())));
	currentBlock->append(std::make_shared<CallAssembly>(currentBlock, P->getMallocFunction()));
	if (m->getPtr() != nullptr)
		currentBlock->append(std::make_shared<MoveAssembly>(currentBlock,
		std::static_pointer_cast<Register>(m->getPtr()), (*P)["a0"]));
}


void InstructionSelector::visit(Return * r)
{
	if (r->getValue() != nullptr) 
		currentBlock->append(std::make_shared<MoveAssembly>(currentBlock,
			(*P)["a0"], toRegister(r->getValue())));
	
	// need somethiing else?
	for (auto it : calleeSaveRegbuckup)
		currentBlock->append(std::make_shared<MoveAssembly>(currentBlock, (*P)[it.first], it.second));
	currentBlock->append(std::make_shared<RetAssembly>(currentBlock));
}

void InstructionSelector::visit(Jump * j)
{
	currentBlock->append(std::make_shared<JumpAssembly>(
		currentBlock, irBlock2RISCV[j->getTarget().get()]));
}

/*************************Helper functions**************************/

void InstructionSelector::functionEntryBlockInit(Function *f, std::shared_ptr<RISCVBasicBlock> newEntry)
{
	currentBlock = newEntry;
	// 1. save callee-save registers to somewhere else (recorded by <backup>)
	calleeSaveRegbuckup.clear();
	auto workList = RISCVConfig::calleeSaveRegNames;
	workList.push_back("ra");
	for (auto &regName : workList) {
		auto reg = std::make_shared<VirtualReg>(VirtualReg::REG_VAL, "buckup_" + regName);
		newEntry->append(std::make_shared<MoveAssembly>(currentBlock, reg, (*P)[regName]));
		calleeSaveRegbuckup[regName] = reg;
	}

	// 2. save argments
	auto args = f->getArgs();
	if (f->getObjRef() != nullptr) 
		args.insert(args.begin(), std::static_pointer_cast<Register>(f->getObjRef()));
	for (int i = 0; i < std::min(8, (int)args.size()); i++)
		currentBlock->append(std::make_shared<MoveAssembly>(currentBlock, args[i], (*P)["a" + std::to_string(i)]));
	for (int i = 8; i < args.size(); i++) {
		auto addr = std::make_shared<StackLocation>(currentFunction, (*P)["sp"], (i - 8)*Configuration::SIZE_OF_INT, false);
		currentBlock->append(std::make_shared<Load>(currentBlock, addr, args[i], Configuration::SIZE_OF_INT));
	}
}

void InstructionSelector::resolveRtype(Quadruple * q)
{
	auto op = q->getOp();
	auto rs1 = toRegister(q->getSrc1());
	auto rs2 = toRegister(q->getSrc2());
	auto rd = std::static_pointer_cast<Register>(q->getDst());
	switch (op)
	{
	case Quadruple::ADD:
		currentBlock->append(std::make_shared<R_type>(currentBlock, R_type::ADD, rd, rs1, rs2));
		break;
	case Quadruple::MINUS:
		currentBlock->append(std::make_shared<R_type>(currentBlock, R_type::SUB, rd, rs1, rs2));
		break;
	case Quadruple::TIMES:
		currentBlock->append(std::make_shared<R_type>(currentBlock, R_type::MUL, rd, rs1, rs2));
		break;
	case Quadruple::DIVIDE:
		currentBlock->append(std::make_shared<R_type>(currentBlock, R_type::DIV, rd, rs1, rs2));
		break;
	case Quadruple::MOD:
		currentBlock->append(std::make_shared<R_type>(currentBlock, R_type::MOD, rd, rs1, rs2));
		break;
	case Quadruple::LESS:
		currentBlock->append(std::make_shared<R_type>(currentBlock, R_type::SLT, rd, rs1, rs2));
		break;
	case Quadruple::LEQ:
	{
		auto temp = std::make_shared<VirtualReg>();
		currentBlock->append(std::make_shared<R_type>(currentBlock, R_type::SLT, temp, rs2, rs1));
		currentBlock->append(std::make_shared<I_type>(currentBlock, I_type::XORI, rd, temp,
			std::make_shared<Immediate>(1)));
		break;
	}
	case Quadruple::GREATER:
		currentBlock->append(std::make_shared<R_type>(currentBlock, R_type::SLT, rd, rs2, rs1));
		break;
	case Quadruple::GEQ:
	{
		auto temp = std::make_shared<VirtualReg>();
		currentBlock->append(std::make_shared<R_type>(currentBlock, R_type::SLT, temp, rs1, rs2));
		currentBlock->append(std::make_shared<I_type>(currentBlock, I_type::XORI, rd, temp,
			std::make_shared<Immediate>(1)));
		break;
	}
	case Quadruple::NEQ:
	{
		auto temp = std::make_shared<VirtualReg>();
		currentBlock->append(std::make_shared<R_type>(currentBlock, R_type::SUB, temp, rs1, rs2));
		currentBlock->append(std::make_shared<R_type>(currentBlock, R_type::SLTU, rd, (*P)["zero"], temp));
		break;
	}
	case Quadruple::EQ:
	{
		auto temp = std::make_shared<VirtualReg>();
		currentBlock->append(std::make_shared<R_type>(currentBlock, R_type::SUB, temp, rs1, rs2));
		currentBlock->append(std::make_shared<I_type>(currentBlock, I_type::SLTIU, rd, temp, 
			std::make_shared<Immediate>(1)));
		break;
	}
	case Quadruple::LSHIFT:
		currentBlock->append(std::make_shared<R_type>(currentBlock, R_type::SLL, rd, rs1, rs2));
		break;
	case Quadruple::RSHIFT:
		currentBlock->append(std::make_shared<R_type>(currentBlock, R_type::SRA, rd, rs1, rs2));
		break;
	case Quadruple::BITAND:
		currentBlock->append(std::make_shared<R_type>(currentBlock, R_type::AND, rd, rs1, rs2));
		break;
	case Quadruple::BITOR:
		currentBlock->append(std::make_shared<R_type>(currentBlock, R_type::OR, rd, rs1, rs2));
		break;
	case Quadruple::BITXOR:
		currentBlock->append(std::make_shared<R_type>(currentBlock, R_type::XOR, rd, rs1, rs2));
	default:
		break;
	}
}

void InstructionSelector::resolveItype(Quadruple * q)
{
	auto op = q->getOp();
	bool swap = q->getSrc1()->category() == Operand::IMM;

	auto rd = std::static_pointer_cast<Register>(q->getDst());
	auto rs1 = toRegister(swap ? q->getSrc2() : q->getSrc1());
	auto imm = std::static_pointer_cast<Immediate>(swap ? q->getSrc1() : q->getSrc2());
	if (swap) op = reverseOp(op);
	switch (op)
	{
	case Quadruple::ADD:
		currentBlock->append(std::make_shared<I_type>(currentBlock, I_type::ADDI, rd, rs1, imm));
		break;
	case Quadruple::MINUS:
	case Quadruple::TIMES:
	case Quadruple::DIVIDE:
	case Quadruple::MOD:
		throw Error("not ready yet.");
	case Quadruple::LESS:
		currentBlock->append(std::make_shared<I_type>(currentBlock, I_type::SLTI, rd, rs1, imm));
		break;
	case Quadruple::LEQ:
	{
		auto temp = std::make_shared<VirtualReg>();
		currentBlock->append(std::make_shared<R_type>(currentBlock, R_type::SLT, temp, toRegister(imm), rs1));
		currentBlock->append(std::make_shared<I_type>(currentBlock, I_type::XORI, rd, temp, 
			std::make_shared<Immediate>(1)));
		break;
	}
	case Quadruple::GREATER:
		currentBlock->append(std::make_shared<R_type>(currentBlock, R_type::SLT, rd, toRegister(imm), rs1));
		break;
	case Quadruple::GEQ:
	{
		auto temp = std::make_shared<VirtualReg>();
		currentBlock->append(std::make_shared<I_type>(currentBlock, I_type::SLTI, temp, rs1, imm));
		currentBlock->append(std::make_shared<I_type>(currentBlock, I_type::XORI, rd, temp,
			std::make_shared<Immediate>(1)));
		break;
	}
	case Quadruple::NEQ:
	{
		auto temp = std::make_shared<VirtualReg>();
		currentBlock->append(std::make_shared<R_type>(currentBlock, R_type::SUB, temp, toRegister(imm), rs1));
		currentBlock->append(std::make_shared<R_type>(currentBlock, R_type::SLTU, rd,(*P)["zero"], temp));
		break;
	}
	case Quadruple::EQ:
	{
		auto temp = std::make_shared<VirtualReg>();
		currentBlock->append(std::make_shared<R_type>(currentBlock, R_type::SUB, temp, toRegister(imm), rs1));
		currentBlock->append(std::make_shared<I_type>(currentBlock, I_type::SLTIU, rd, temp, 
			std::make_shared<Immediate>(1)));
		break;
	}
	case Quadruple::LSHIFT:
		currentBlock->append(std::make_shared<I_type>(currentBlock, I_type::SLLI, rd, rs1, imm));
		break;
	case Quadruple::RSHIFT:
		currentBlock->append(std::make_shared<I_type>(currentBlock, I_type::SRAI, rd, rs1, imm));
		break;
	case Quadruple::BITAND:
		currentBlock->append(std::make_shared<I_type>(currentBlock, I_type::ANDI, rd, rs1, imm));
		break;
	case Quadruple::BITOR:
		currentBlock->append(std::make_shared<I_type>(currentBlock, I_type::ORI, rd, rs1, imm));
		break;
	case Quadruple::BITXOR:
		currentBlock->append(std::make_shared<I_type>(currentBlock, I_type::XORI, rd, rs1, imm));
		break;
	default:
		break;
	}
}

bool InstructionSelector::isRtype(std::shared_ptr<Operand> lhs, Quadruple::Operator op, std::shared_ptr<Operand> rhs)
{
	static const std::unordered_set<Quadruple::Operator> ops = 
	{ Quadruple::MOD, Quadruple::MINUS, Quadruple::TIMES, Quadruple::DIVIDE };
	if (ops.find(op) != ops.end()) return true;
	if (Operand::isRegister(lhs->category()) && Operand::isRegister(rhs->category())) return true;
	if (lhs->category() == Operand::IMM && !isInRange(std::static_pointer_cast<Immediate>(lhs))) return true;
	if (rhs->category() == Operand::IMM && !isInRange(std::static_pointer_cast<Immediate>(rhs))) return true;
	return (op == Quadruple::LSHIFT || op == Quadruple::RSHIFT) && lhs->category() == Operand::IMM;
}

std::shared_ptr<Register> InstructionSelector::toRegister(std::shared_ptr<Operand> x)
{
	auto c = x->category();
	if (Operand::isRegister(c)) return std::static_pointer_cast<Register>(x);
	if (c == Operand::STATICSTR) {
		auto reg = std::make_shared<VirtualReg>();
		currentBlock->append(std::make_shared<LoadAddr>(currentBlock, 
			reg,std::static_pointer_cast<StaticString>(x)));
		return reg;
	}
	if (c == Operand::IMM) {
		auto reg = std::make_shared<VirtualReg>();
		currentBlock->append(std::make_shared<LoadImm>(currentBlock, 
			reg, std::static_pointer_cast<Immediate>(x)));
		return reg;
	}
	throw Error("What the hell are you doing?");
}

bool InstructionSelector::isInRange(std::shared_ptr<Immediate> i)
{
	static const int B = 11;
	return -(1 << 11) <= i->getValue() && i->getValue() <= ((1 << 11) - 1);
}

Quadruple::Operator InstructionSelector::reverseOp(Quadruple::Operator op)
{
	static const std::unordered_map<Quadruple::Operator, Quadruple::Operator> pairs =
	{
		{Quadruple::LESS,Quadruple::GREATER},
		{Quadruple::LEQ,Quadruple::GEQ},
		{Quadruple::GREATER,Quadruple::LESS},
		{Quadruple::GEQ,Quadruple::LEQ},
	};
	auto it = pairs.find(op);
	return it == pairs.end() ? op : it->second;
}
