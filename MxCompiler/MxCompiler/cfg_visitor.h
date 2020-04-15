#pragma once

#include "pch.h"	

#define ACCEPT_CFG_VISITOR void accept (CFG_Visitor &vis) {vis.visit(this);} 

class IR;
class Function;
class BasicBlock;
class Quadruple;
class Branch;
class Call;
class Malloc;
class Return;
class Jump;
class PhiFunction;
class Register;
class StaticString;
class Immediate;

class CFG_Visitor {
public:
	virtual void visit(IR *ir) {}

	virtual void visit(Function *f) {}

	virtual void visit(BasicBlock *b) {}

	/*Instructions*/
	virtual void visit(Quadruple *q) {}

	virtual void visit(Branch *b) {}

	virtual void visit(Call *c) {}

	virtual void visit(Malloc *m) {}

	virtual void visit(Return *r) {}

	virtual void visit(Jump *j){}

	virtual void visit(PhiFunction *p){}

	/*For Operands*/
	virtual void visit(Register *r) {}
	virtual void visit(StaticString *s){}
	virtual void visit(Immediate *i){}
};