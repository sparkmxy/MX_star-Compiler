#pragma once

#include "pch.h"	
#include "IR.h"
#include "Function.h"
#include "basicblock.h"

class CFG_Vistor {
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
};