#include "IR_Generator.h"

void IR_Generator::visit(ProgramAST * node)
{
	auto decls = node->getDecls();
	scanGlobalVar = true;
	for (auto &decl : decls)
		if(decl->isVarDecl())decl->accept(*this);
	scanGlobalVar = false;
	for (auto &decl : decls)
		if (!decl->isVarDecl()) decl->accept(*this);
}

void IR_Generator::visit(MultiVarDecl * node)
{
	auto vars = node->getDecls();
	for (auto &var : vars) var->accept(*this);
}

void IR_Generator::visit(VarDeclStmt * node)
{
	auto varSymbol = node->getVarSymbol();
	auto reg = std::make_shared<Int64Global>(node->getIdentifier()->name);
	varSymbol->setReg(reg);
	if (scanGlobalVar)  // global variable
		ir->addGlobalVar(reg);
	else {
		if (currentFunction != nullptr && varSymbol->isArgument())
			currentFunction->appendArg(reg);
		auto initExpr = node->getInitExpr();
		if (initExpr != nullptr) {
			initExpr->accept(*this);
			
		}
	}
}

void IR_Generator::visit(FunctionDecl * node)
{
}

void IR_Generator::visit(ClassDecl * node)
{
}

