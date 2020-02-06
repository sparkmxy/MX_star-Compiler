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
}

void SymbolTable::visit(VarDeclStmt *node) {
	if (node->getInitExpr() != nullptr)
		node->getInitExpr()->accept(*this);
	std::shared_ptr<SymbolType> type = symbolTypeOfNode(node->getType().get(), globalScope);
	node->setSymbolType(type);
	auto var = std::make_shared<VarSymbol>(node->getIdentifier()->name, type, node);
	node->setVarSymbol(var);
	currentScope->define(var);
	//need to check whether currentScope == globalScope ?
}

void SymbolTable::visit(ExprStmt * node)
{
	node->getExpr()->accept(*this);
}

void SymbolTable::visit(FuncCallExpr * node)
{
	auto funcSymbol = std::static_pointer_cast<FunctionSymbol>(currentScope->resolve(node->getIdentifier()->name));
	node->setFuncSymbol(funcSymbol);
	auto args = node->getArgs();
	for (auto &arg : args) arg->accept(*this);
}

void SymbolTable::visit(MemberFuncCallExpr * node)
{
	node->getInstance()->accept(*this);
	auto funcSymbol = std::static_pointer_cast<FunctionSymbol>(currentScope->resolve(node->getIdentifier()->name));
	node->setFuncSymbol(funcSymbol);
	auto args = node->getArgs();
	for (auto &arg : args) arg->accept(*this);
}

void SymbolTable::visit(IdentifierExpr * node)
{
	auto symbol = currentScope->resolve(node->getIdentifier()->name);
	node->setSymbol(symbol);
}

void SymbolTable::visit(NewExpr * node)
{
	auto type = symbolTypeOfNode(node->getBaseType().get(), globalScope);
	node->setSymbolType(type);
	auto dims = node->getDimensions();
	for (auto &dim : dims) dim->accept(*this);
}

void SymbolTable::visit(UnaryExpr * node)
{
	node->getOprand()->accept(*this);
}

void SymbolTable::visit(BinaryExpr * node)
{
	node->getLHS()->accept(*this);
	node->getRHS()->accept(*this);
}

void SymbolTable::visit(FunctionDecl *node) {
	std::shared_ptr<SymbolType> retType = symbolTypeOfNode(node->getRetType().get(), globalScope);
	// it can be a constructor
	bool isConstructor = false;
	if (currentClassSymbol != nullptr && node->getIdentifier == "::ctor") {
		if (currentClassSymbol->getConstructor() != nullptr)
			throw SemanticError("Duplicated constructor.", node->Where());
		isConstructor = true;
		retType = currentClassSymbol;
	}
	auto funcSymbol = std::make_shared<FunctionSymbol>(node->getIdentifier()->name,retType, node, currentScope);
	if (isConstructor)
		currentClassSymbol->setConstructor(funcSymbol);
	node->setFuncSymbol(funcSymbol);
	auto args = node->getArgs();
	for (auto &varDeclStmt : args) {
		auto argType = symbolTypeOfNode(varDeclStmt->getType().get(), globalScope);
		auto varSymbol = std::make_shared<VarSymbol>(varDeclStmt->getIdentifier()->name, argType, node);
		varDeclStmt->setVarSymbol(varSymbol);
		funcSymbol->define(varSymbol);
	}
	currentScope->define(funcSymbol);

	currentScope = funcSymbol;
	currentFunctionSymbol = funcSymbol;
	node->getBody()->accept(*this);
}

void SymbolTable::visit(ClassDecl *node)
{
	auto clsSymbol = std::make_shared<ClassSymbol>(node->getIdentifier()->name, node, globalScope);
	globalScope->define(clsSymbol);
	node->setClsSymbol(clsSymbol);
	//maintian pointers
	currentScope = clsSymbol;
	currentClassSymbol = clsSymbol;

	//deal with members
	auto members = node->getMembers();
	for (auto &decl : members) {
		decl->accept(*this);
		currentScope = currentClassSymbol;
	}
}

void SymbolTable::visit(StmtBlock * node)
{
	auto local = std::make_shared<LocalScope>("just a name", currentScope);
	currentScope = local;

	auto stmts = node->getStmts();
	for (auto stmt : stmts) {
		stmt->accept(*this);
		currentScope = local;
	}
}

void SymbolTable::visit(IfStmt * node)
{
	node->getCondition()->accept(*this);
	node->getThen()->accept(*this);
	auto Else = node->getElse();
	if (Else != nullptr) Else->accept(*this);
}

void SymbolTable::visit(WhileStmt * node)
{
	loops.push(node);
	node->getCondition()->accept(*this);
	node->getBody()->accept(*this);
	loops.pop();
}

void SymbolTable::visit(ForStmt * node)
{
	loops.push(node);
	if (node->getInit != nullptr) node->getInit()->accept(*this);
	if (node->getCondition() != nullptr) node->getCondition()->accept(*this);
	if (node->getIter() != nullptr) node->getIter()->accept(*this);
	node->getBody()->accept(*this);
	loops.pop();
}

void SymbolTable::visit(ReturnStmt * node)
{
	if (node->getValue() != nullptr)
		node->getValue()->accept(*this);
	if (currentFunctionSymbol == nullptr)
		throw SemanticError("'return' must be in a function.", node->Where());
	node->setFuncSymbol(currentFunctionSymbol);
}

void SymbolTable::visit(BreakStmt * node)
{
	if (loops.empty())
		throw new SemanticError("'break' must be in a loop.", node->Where());
	node->setLoop(loops.top());
}




