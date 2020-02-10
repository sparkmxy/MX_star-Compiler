#pragma once

#include "pch.h"
#include "visitor.h"
#include "astnode.h"
#include "symbol.h"

class ClassDeclVisitor : public Visitor {
public:
	ClassDeclVisitor(std::shared_ptr<GlobalScope> _globalScope)
		:globalScope(_globalScope) {}

	void visit(ProgramAST *node) override;
private:
	void visit(ClassDecl *node) override;

	std::shared_ptr<GlobalScope> globalScope;
};