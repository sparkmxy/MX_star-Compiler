#include "symbolTable.h"

void SymbolTable::visit(ProgramAST *program) {
	auto decls = program->getDecls();
	for (auto &decl : decls) {
		currentScope = globalScope;
		currentClassSymbol = nullptr;
		currentFunctionSymbol = nullptr;
		decl->accept(*this);
	}
}


void SymbolTable::visit(VarDecl *node) {
	node->getStmt()->accept(*this);
	auto stmt = node->getStmt();
	std::shared_ptr<SymbolType> type = symbolTypeOfNode(stmt->getType().get(), globalScope);
	node->setSymbolType(type);
	auto var = std::make_shared<VarSymbol>(stmt->getIdentifier()->name, type, node);

	// not finish yet
}

void SymbolTable::visit(VarDeclStmt *node) {
	if (node->getInitExpr() != nullptr)
		node->getInitExpr()->accept(*this);
}

void SymbolTable::visit(FunctionDecl *node) {
	//check return type (nullptr stands for void)
	auto retType = symbolTypeOfNode(node->getRetType().get(), globalScope);
	auto funcSymbol = std::make_shared<FunctionSymbol>(node->getIdentifier()->name,retType, node, globalScope);
	node->setFuncSymbol(funcSymbol);
	auto args = node->getArgs();
	for (auto &varDeclStmt : args) {
		auto argType = symbolTypeOfNode(varDeclStmt->getType().get(), globalScope);
		auto varSymbol = std::make_shared<VarSymbol>(varDeclStmt->getIdentifier()->name, argType, node);
		varDeclStmt->setVarSymbol(varSymbol);
		funcSymbol->define(varSymbol);
	}
	globalScope->define(funcSymbol);

	currentScope = funcSymbol;
	currentFunctionSymbol = funcSymbol;
	node->getBody()->accept(*this);
}

void SymbolTable::visit(ClassDecl *node)
{
	auto clsSymbol = std::make_shared<ClassSymbol>(node->getIdentifier()->name, node, globalScope);
	globalScope->define(clsSymbol);
	node->setClsSymbol(clsSymbol);
	//deal with members

}
