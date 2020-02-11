#pragma once

#include "pch.h"
#include "visitor.h"
#include "astnode.h"
#include "symbol.h"

class GlobalFuncAndClsVisitor : public Visitor{
public:
	GlobalFuncAndClsVisitor(std::shared_ptr<GlobalScope> _globalScope)
		:globalScope(_globalScope) {}

	void visit(ProgramAST *node) override;
private:
	void visit(FunctionDecl *node) override;
	void visit(ClassDecl *node) override;
	void visit(VarDeclStmt *node) override;
	void visit(MultiVarDecl *node) override;

	std::shared_ptr<GlobalScope> globalScope;
	std::shared_ptr<Scope> currentScope;
	std::shared_ptr<ClassSymbol> currentClassSymbol;
};