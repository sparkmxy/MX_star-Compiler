#include "ClassDeclVisitor.h"

void ClassDeclVisitor::visit(ProgramAST * node)
{
	auto decls = node->getDecls();
	for (auto decl : decls) decl->accept(*this);
}

void ClassDeclVisitor::visit(ClassDecl * node)
{
	auto clsSymbol =
		std::make_shared<ClassSymbol>(node->getIdentifier()->name, node, globalScope);
	globalScope->define(clsSymbol);
	node->setClsSymbol(clsSymbol);
}
