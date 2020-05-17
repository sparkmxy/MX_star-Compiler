#pragma once

#include "pch.h"
#include "IR.h"
#include "cfg_visitor.h"
#include "basicblock.h"
#include "RISCVassembly.h"
#include "RISCVinstruction.h"
#include "configuration.h"


/*
Translate IR into RISCV assembly with infinte number of registers
*/
class InstructionSelector :public CFG_Visitor{
public:
	InstructionSelector(std::shared_ptr<IR> _ir);
	std::shared_ptr<RISCVProgram> getRISCVProgram() { return P; }

private:
	std::shared_ptr<IR> ir;
	std::shared_ptr<RISCVProgram> P;

	std::unordered_map<Function *, std::shared_ptr<RISCVFunction> > irFunc2RISCV;
	std::unordered_map<BasicBlock *, std::shared_ptr<RISCVBasicBlock> > irBlock2RISCV;

	std::shared_ptr<RISCVBasicBlock> currentBlock;
	std::shared_ptr<RISCVFunction> currentFunction;

	std::unordered_map<std::string, int> labelCnt;
	std::string getLabel(const std::string &label);

	std::unordered_map<std::string, std::shared_ptr<VirtualReg> > calleeSaveRegbuckup;

	// override functons 
	void visit(IR *ir) override;
	void visit(Function *f) override;
	void visit(BasicBlock *b) override;

	void visit(Quadruple *q) override;
	void visit(Branch *b) override;
	void visit(Call *c) override;
	void visit(Malloc *m) override;
	void visit(Return *r) override;
	void visit(Jump *j) override;

	// Helper functions
	void functionEntryBlockInit(Function *f, std::shared_ptr<RISCVBasicBlock> newEntry);

	void resolveRtype(Quadruple *q);
	void resolveItype(Quadruple *q);

	bool isRtype(std::shared_ptr<Operand> lhs, Quadruple::Operator op, std::shared_ptr<Operand> rhs);
	std::shared_ptr<Register> toRegister(std::shared_ptr<Operand> x);
	bool isInRange(std::shared_ptr<Immediate> i);
	Quadruple::Operator reverseOp(Quadruple::Operator op);
};