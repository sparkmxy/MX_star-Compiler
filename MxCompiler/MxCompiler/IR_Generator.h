#pragma once

#include "pch.h"
#include "astnode.h"
#include "visitor.h"
#include "configuration.h"

#include "Function.h"
#include "IR.h"
#include "basicblock.h"

/*
Usage : generator = new IR_Generator(globalScope);
*/
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
	void visit(ForStmt *node) override;
	void visit(WhileStmt *node) override;
	void visit(IfStmt *node) override;
	void visit(ReturnStmt *node) override;
	void visit(ContinueStmt *node) override;
	void visit(BreakStmt *node) override;
	// EmptyStmt is ignored
	void visit(IdentifierExpr *node) override;
	void visit(BinaryExpr *node) override;
	void visit(UnaryExpr *node) override;
	void visit(FuncCallExpr *node) override;
	void visit(MemberFuncCallExpr *node) override;
	void visit(ClassMemberExpr *node) override;
	void visit(ThisExpr *node) override;
	void visit(NewExpr *node) override;

	void visit(NumValue *node) override;
	void visit(BoolValue *node) override;
	void visit(StringValue *node) override;
	void visit(NullValue *node) override;

	/*Helper Functions*/ 
	void newArray(NewExpr *node, std::shared_ptr<Operand> addrReg, int dimension = 0);
	std::shared_ptr<Operand>
		allocateMemory(std::shared_ptr<Operand> addrReg, std::shared_ptr<Operand> size);

	// for binary exprssions
	void arraySize(MemberFuncCallExpr *node);

	void arrayAccess(BinaryExpr *node);

	void boolOnlyExpr(BinaryExpr *node);

	void assign(std::shared_ptr<Operand> lhs, Expression * rhs);

	void assignBool(std::shared_ptr<Operand> lhs, Expression * rhs);

	static bool isBoolOnlyOperator(BinaryExpr::Operator op);

	std::shared_ptr<Operand> getValueReg(std::shared_ptr<Operand> reg);
};
