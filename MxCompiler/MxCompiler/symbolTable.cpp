#include "symbolTable.h"

void SymbolTable::visit(ProgramAST &program) {
	auto decls = program.getDecls();
	for (auto &decl : decls) {
		currentScope = globalScope;
		currentClassSymbol = nullptr;
		currentFunctionSymbol = nullptr;
		decl->accept(*this);
	}
}


void SymbolTable::visit(VarDecl &node) {
	node.getStmt()->accept(*this);
	auto stmt = node.getStmt();
	std::shared_ptr<SymbolType> type = SymbolTypeOfNode(*stmt->getType(), globalScope);
	node.setSymbolType(type);
	auto var = std::make_shared<VarSymbol>(stmt->getIdentifier()->name, type, node);
}

void SymbolTable::visit(VarDeclStmt &node) {
	if (node.getInitExpr() != nullptr)
		node.getInitExpr()->accept(*this);
}

void SymbolTable::visit(FunctionDecl &node) {
	
}
