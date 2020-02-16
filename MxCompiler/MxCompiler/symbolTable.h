#pragma once
#include "pch.h"
#include "visitor.h"
#include "astnode.h"
#include "symbol.h"


class SymbolTable : Visitor{
public:
	SymbolTable(std::shared_ptr<GlobalScope> _globalScope) 
		:globalScope(_globalScope){}

	void visit(ProgramAST *node) override;

private:
	std::shared_ptr<GlobalScope> globalScope;
	std::shared_ptr<Scope> currentScope;
	std::shared_ptr<ClassSymbol> currentClassSymbol;
	std::shared_ptr<FunctionSymbol> currentFunctionSymbol;

	//This is for <break> and <continue> statement 
	std::stack<Loop *> loops;

	//override functions
	
	void visit(MultiVarDecl *node) override;
	void visit(FunctionDecl *node)override;
	void visit(ClassDecl *node)override;
	
	void visit(StmtBlock *node)override;
	void visit(IfStmt *node)override;
	void visit(WhileStmt *node)override;
	void visit(ForStmt *node)override;
	void visit(ReturnStmt *node) override;
	void visit(BreakStmt *node) override;
	void visit(VarDeclStmt *node) override;
	void visit(ExprStmt *node) override;

	void visit(FuncCallExpr *node) override;
	void visit(ClassMemberExpr *node) override;
	void visit(MemberFuncCallExpr *node) override;
	void visit(IdentifierExpr *node) override;
	void visit(NewExpr *node) override;
	void visit(UnaryExpr *node) override;
	void visit(BinaryExpr *node) override;
	void visit(ThisExpr *node) override;

private:
	void checkMainFunc();
};