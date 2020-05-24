#include "RISCVcodegen.h"


void RISCVCodeGenerator::generate()
{
	auto instructionSelector = InstructionSelector(ir);
	riscv_program = instructionSelector.getRISCVProgram();
}

void RISCVCodeGenerator::emit()
{

	os << '\t' << ".section\t.data\n";
	for(auto var : ir->getGlbVars()){
		os << '\t' << ".globl\t" + var->getName() << '\n';
		os << var->getName() << ":\n";
		os << '\t' << ".zero\t4\n\n";
	}

	for (auto str : ir->getStringConstants()) {
		os << '\t' << ".globl\t" << str->getReg()->getName() << '\n';
		os << str->getReg()->getName() << ":\n";
		os << ".string\t" + str->getText() << "\n\n";
	}

	os << '\t' << ".text\n";

	for (auto f : riscv_program->getFunctions()) emitFunction(f);
}

void RISCVCodeGenerator::emitFunction(std::shared_ptr<RISCVFunction> f)
{
	int cnt = 0;
	for (auto b : f->getBlockList()) label[b] = f->getName() + "_" + std::to_string(cnt++);
	for (auto b : f->getBlockList()) {
		os <<  label[b] << ":\n";
		emitBlock(b);
	}
}

void RISCVCodeGenerator::emitBlock(std::shared_ptr<RISCVBasicBlock> b)
{
	for (auto i = b->getFront(); i != nullptr; i = i->getNextInstr()) {
		emitInstruction(i);
		os << '\n';
	}
}

void RISCVCodeGenerator::emitInstruction(std::shared_ptr<RISCVinstruction> i)
{
	auto c = i->category();
	auto str = i->toString();
	if (c == RISCVinstruction::JUMP) 
		str = str + ", " + label[std::static_pointer_cast<JumpAssembly>(i)->getTarget()];
	else if (c == RISCVinstruction::BTYPE) 
		str = str + "," + label[std::static_pointer_cast<B_type>(i)->getTargetBlock()];
	else if (c == RISCVinstruction::LOAD) 
		str = str + ", " + addr2String(std::static_pointer_cast<Load>(i)->getAddr());
	else if (c == RISCVinstruction::STORE) {
		auto s = std::static_pointer_cast<Store>(i);
		str = str + ", " + addr2String(s->getAddr());
		if (s->getRt() != nullptr) str = str + ", " + s->getRt()->getName();
	}
	os << '\t' << str;
}

std::string RISCVCodeGenerator::addr2String(std::shared_ptr<Address> addr)
{
	if (addr->category() == Operand::STACK) {
		auto a = std::static_pointer_cast<StackLocation>(addr);
		int offset;
		if (a->isTopdown()) offset = a->getOffset();
		else offset = a->getFunction()->getStackSize() - a->getOffset();
		return std::to_string(offset) + "(sp)";
	}
	else {
		auto a = std::static_pointer_cast<BaseOffsetAddr>(addr);
		return std::to_string(a->getOffset()) + '(' + a->getBase()->getName() + ')';
	}
}
