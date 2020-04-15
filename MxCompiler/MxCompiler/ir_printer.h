#pragma once 
#include "pch.h"
#include "IR.h"
#include "cfg_visitor.h"
#include "basicblock.h"
#include "IRinstruction.h"

class IR_Printer : public CFG_Visitor{
public:
	IR_Printer(std::shared_ptr<IR> _ir, std::ostream &_os = std::cerr) 
		:ir(_ir),os(_os){}

	void print();
private:
	std::shared_ptr<IR> ir;
	std::ostream &os;

	/*Override functions*/
	void visit(IR *ir) override;
	void visit(Function *f) override;
	void visit(BasicBlock *b) override;

	void visit(Quadruple *q) override;
	void visit(Branch *b) override;
	void visit(Call *c) override;
	void visit(Malloc *m) override;
	void visit(Return *r) override;
	void visit(Jump *j) override;
	void visit(PhiFunction *p) override;
	void visit(Register *r) override;
	void visit(StaticString *s) override;
	void visit(Immediate *i) override;

	
	std::unordered_map<std::string, int> nameCounter;

	std::unordered_map<Register *, std::string> nameForReg;
	std::string getName(Register *reg);
	std::string newRegName(Register *reg);

	std::unordered_map<BasicBlock *, std::string> label;
	std::string getLabel(BasicBlock *block);
	std::string newLabel(BasicBlock *block);
};