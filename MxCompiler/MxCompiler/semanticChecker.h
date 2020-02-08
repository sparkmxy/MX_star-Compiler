#pragma once
#include "pch.h"
#include "scope.h"
#include "astnode.h"
#include "visitor.h"

class SemanticChecker : public Visitor {
public:
	SemanticChecker(std::shared_ptr<GlobalScope> _globalScope)
		:globalScope(_globalScope){
		intSymbol = globalScope->getIntSymbol();
		boolSymbol = globalScope->getBoolSymbol();
		stringClassSymbol = globalScope->getStringSymbol();
		voidSymbol = globalScope->getVoidSymbol();
	}

	void visit(ProgramAST *node) override;

private:

	//override functions
	void visit(VarDecl *node) override;
	void visit(FunctionDecl *node) override;
	void visit(ClassDecl *node) override;

	void visit(VarDeclStmt *node) override;
	void visit(StmtBlock *node) override;
	void visit(ExprStmt *node) override;
	void visit(IfStmt *node) override;
	void visit(WhileStmt *node) override;
	void visit(ForStmt *node) override;
	void visit(ReturnStmt *node) override;
	// break : do nothing
	//continue : do nothing
	
	void visit(BinaryExpr *node) override;
	void visit(ClassMemberExpr *node) override;
	void visit(MemberFuncCallExpr *node) override;
	void visit(FuncCallExpr *node) override;
	void visit(IdentifierExpr *node) override;
	void visit(NewExpr *node)override;
	void visit(UnaryExpr *node) override;

	void visit(StringValue *node) override;
	void visit(NumValue *node) override;
	void visit(BoolValue *node) override;
	void visit(NullValue *node) override;
	/*Helper Functions*/
	bool isBoolOnlyOperator(BinaryExpr::Operator op);

	/*Builtin Symbols*/
private:
	std::shared_ptr<GlobalScope> globalScope;
	std::shared_ptr<BuiltInTypeSymbol> intSymbol;
	std::shared_ptr<BuiltInTypeSymbol> boolSymbol;
	std::shared_ptr<BuiltInTypeSymbol> voidSymbol;
	std::shared_ptr<ClassSymbol> stringClassSymbol;
};

