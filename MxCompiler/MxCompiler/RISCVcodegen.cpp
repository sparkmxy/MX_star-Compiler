#include "RISCVcodegen.h"
#include "peeholeMatching.h"

void RISCVCodeGenerator::generate()
{
	auto instructionSelector = InstructionSelector(ir);
	riscv_program = instructionSelector.getRISCVProgram();
	std::cout << "assembly code generation is completed.\n";
	auto regalloc = RegisterAllocator(riscv_program);
	regalloc.run();

	std::make_shared<PeeholeMatchingOptimizer>(riscv_program)->run();
	setSP();
	renameMainFunction();
	std::cout << "register allocation is completed\n";
}

void RISCVCodeGenerator::emit()
{

	os << '\t' << ".section\t.data\n";
	for(auto var : ir->getGlbVars()){
		os << '\t' << ".globl\t" + var->getName() << '\n';
		os << var->getName() << ":\n";
		os << '\t' << ".zero\t4\n\n";
	}

	int cnt = 0;
	for (auto str : ir->getStringConstants()) {
		stringLabel[str->getReg()] = cnt++;
		auto name = "str" + std::to_string(stringLabel[str->getReg()]);
		os << '\t' << ".globl\t" << name << '\n';
		os << name << ":\n";
		os << "\t.string\t" + str->getText() << "\n\n";
	}

	os << '\t' << ".text\n";

	for (auto f : riscv_program->getFunctions()) emitFunction(f);
}

void RISCVCodeGenerator::setSP()
{
	for (auto &f : riscv_program->getFunctions()) {
		int stackSize = f->getStackSize() + 4;
		if (stackSize > 0) {
			auto sp = (*riscv_program)["sp"];
			appendBefore(f->getEntry()->getFront(), std::make_shared<I_type>(
				f->getEntry(), I_type::ADDI, sp, sp, std::make_shared<Immediate>(-stackSize)));
			appendBefore(f->getExit()->getBack(), std::make_shared<I_type>(
				f->getExit(), I_type::ADDI, sp, sp, std::make_shared<Immediate>(stackSize)));
		}
	}
}

void RISCVCodeGenerator::renameMainFunction()
{
	for (auto &f : riscv_program->getFunctions())
		if (f->getName() == "main") f->setName("_main");
		else if (f->getName() == "__bootstrap") f->setName("main");
}

void RISCVCodeGenerator::emitFunction(std::shared_ptr<RISCVFunction> f)
{
	os << "\t.globl\t" << f->getName() << '\n';
	os << f->getName() << ":\n";
	int cnt = 0;
	for (auto b : f->getBlockList()) label[b] = "." + f->getName() + "_" + std::to_string(cnt++);
	for (auto b : f->getBlockList()) {
		os << label[b] << ":\n";
		emitBlock(b);
	}
	os << '\n';
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
		str += label[std::static_pointer_cast<JumpAssembly>(i)->getTarget()];
	else if (c == RISCVinstruction::BTYPE)
		str = str + ", " + label[std::static_pointer_cast<B_type>(i)->getTargetBlock()];
	else if (c == RISCVinstruction::LOAD)
		str = str + ", " + addr2String(std::static_pointer_cast<Load>(i)->getAddr());
	else if (c == RISCVinstruction::STORE) {
		auto s = std::static_pointer_cast<Store>(i);
		str = str + ", " + addr2String(s->getAddr());
		if (s->getRt() != nullptr) str = str + ", " + s->getRt()->getName();
	}
	else if (c == RISCVinstruction::CALL) {
		str += std::static_pointer_cast<CallAssembly>(i)->getFunction()->getName();
	}
	else if (c == RISCVinstruction::LA)
		str += "str" + std::to_string(stringLabel[std::static_pointer_cast<LoadAddr>(i)->getSymbol()->getReg()]);
	os << '\t' << str;
}

std::string RISCVCodeGenerator::addr2String(std::shared_ptr<Address> addr)
{
	if (addr->category() == Operand::STACK) {
		auto a = std::static_pointer_cast<StackLocation>(addr);
		return std::to_string(a->getOffset()) + "(sp)";
	}
	else {
		auto a = std::static_pointer_cast<BaseOffsetAddr>(addr);
		if(a->getBase()->category() == Operand::PHISICAL)
			return std::to_string(a->getOffset()) + '(' + a->getBase()->getName() + ')';
		else {  // global
			return a->getBase()->getName();
		}
	}
}
