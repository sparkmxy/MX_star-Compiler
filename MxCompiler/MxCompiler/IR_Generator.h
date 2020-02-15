#pragma once

#include "pch.h"
#include "astnode.h"
#include "visitor.h"
#include "configuration.h"

#include "Function.h"
#include "IR.h"
#include "basicblock.h"

class IR_Generator : public Visitor {
public:

	IR_Generator(std::shared_ptr<GlobalScope> _global) : global(_global) {}

	std::shared_ptr<IR> getIR() { return ir; }

	void visit(ProgramAST *node) override;

private:
	std::shared_ptr<IR> ir;
	std::shared_ptr<GlobalScope> global;

	bool scanGlobalVar;
	std::shared_ptr<Function> currentFunction;
	std::shared_ptr<ClassSymbol> currentClsSymbol;
	std::shared_ptr<BasicBlock> currentBlock;
	// override visitor functions 

	void visit(MultiVarDecl *node) override;
	void visit(VarDeclStmt *node) override;
	void visit(FunctionDecl *node) override;
	void visit(ClassDecl *node) override;

	void visit(StmtBlock *node) override;
	void visit(ExprStmt *node) override;

	void visit(BinaryExpr *node) override;
	void visit(UnaryExpr *node) override;
	/*Helper Functions*/
	// for binary exprssions
	void arrayAccess(BinaryExpr *node);
	void boolOnlyExpr(BinaryExpr *node);
	void assign(std::shared_ptr<Operand> lhs, Expression * rhs);
	void assignBool(std::shared_ptr<Operand> lhs, Expression * rhs);
	static bool isBoolOnlyOperator(BinaryExpr::Operator op);
	std::shared_ptr<Operand> getValueReg(std::shared_ptr<Operand> reg);
};
