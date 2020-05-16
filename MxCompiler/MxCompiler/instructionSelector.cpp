#include "instructionSelector.h"

InstructionSelector::InstructionSelector(std::shared_ptr<IR> _ir):ir(_ir)
{
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
	auto functions = ir->getFunctions();
	for (auto &f : functions) f->accept(*this);
}

void InstructionSelector::visit(Function * f)
{
	auto ff = std::make_shared<RISCVFunction>(f->getName());
	irFunc2RISCV[f] = ff;
	
	currentFunction = ff;
	auto blocks = f->getBlockList();
	for (auto &b : blocks) b->accept(*this);
}

void InstructionSelector::visit(BasicBlock * b)
{
	auto bb = std::make_shared<RISCVBasicBlock>(
		getLabel(currentFunction->getName() + "_" + b->toString()));
	currentBlock = bb;
	irBlock2RISCV[b] = bb;

	for (auto i = b->getFront(); i != nullptr; i = i->getNextInstr())
		i->accept(*this);
}

void InstructionSelector::visit(Quadruple * q)
{
	auto op = q->getOp();
	if (op == Quadruple::STORE) {

	}
	else if (op == Quadruple::LOAD) {
	}
	else if (op == Quadruple::MOVE) {

	}
	else if (op == Quadruple::NEG) {

	}
	else if (op == Quadruple::INV) {

	}
	else {
		if (isRtype(q->getSrc1(), op, q->getSrc2())) resolveRtype(q);
		else resolveItype(q);
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
		currentBlock->append(std::make_shared<R_type>(currentBlock, R_type::SLT, rd, rs2, rs1));
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
		currentBlock->append(std::make_shared<R_type>(currentBlock, R_type::SLT, rd, rs1, rs2));
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
	auto rs1 = std::static_pointer_cast<Register>(swap ? q->getSrc2() : q->getSrc1());
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
		currentBlock->append(std::make_shared<I_type>(currentBlock, I_type::SLTIU, rd,(*P)["zero"], rs1));
		break;
	}
	case Quadruple::EQ:
	{
		auto temp = std::make_shared<VirtualReg>();
		currentBlock->append(std::make_shared<R_type>(currentBlock, R_type::SUB, temp, toRegister(imm), rs1));
		currentBlock->append(std::make_shared<I_type>(currentBlock, I_type::SLTIU, rd, rs1, 
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
	if (Operand::isRegister(lhs->category()) && Operand::isRegister(rhs->isRegister)) return true;
	if (lhs->category() == Operand::IMM && !isInRange(std::static_pointer_cast<Immediate>(lhs))) return true;
	if (rhs->category() == Operand::IMM && !isInRange(std::static_pointer_cast<Immediate>(rhs))) return true;
	return (op == Quadruple::LSHIFT || op == Quadruple::RSHIFT) && lhs->category() == Operand::IMM;
}

std::shared_ptr<Register> InstructionSelector::toRegister(std::shared_ptr<Operand> x)
{
	auto c = x->category();
	if (c == Operand::REG_REF || c == Operand::REG_VAL) return std::static_pointer_cast<Register>(x);
	if (c == Operand::STATICSTR) {
		auto reg = std::make_shared<VirtualReg>();
		currentBlock->append(std::make_shared<LoadAddr>(reg,std::static_pointer_cast<StaticString>(x)));
		return reg;
	}
	if (c == Operand::IMM) {
		auto reg = std::make_shared<VirtualReg>();
		currentBlock->append(std::make_shared<LoadImm>(reg, std::static_pointer_cast<Immediate>(x)));
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
