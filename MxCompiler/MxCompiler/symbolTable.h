#pragma once

#include "pch.h"
#include "symbol.h"

class SymbolTable : Visitor{
public:
	SymbolTable();
private:
	std::shared_ptr<GlobalScope> globalScope;
	std::shared_ptr<Scope> currentScope;
	std::shared_ptr<ClassSymbol> currentClassSymbol;
	std::shared_ptr<FunctionSymbol> currentFunctionSymbol;

	//This is for <break> and <continue> statement 
	std::stack<std::shared_ptr<Statement> > loops;
	void visit(ProgramAST *node) override;
	void visit(VarDecl *node) override;
	void visit(VarDeclStmt *node) override;
	void visit(FunctionDecl *node)override;
	void visit(ClassDecl *node)override;
};