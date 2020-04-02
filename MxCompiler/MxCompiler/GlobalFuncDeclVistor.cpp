#include "GlobalFuncAndClsDecl.h"

void GlobalFuncAndClsVisitor::visit(ProgramAST * node)
{
	auto decls = node->getDecls();
	for (auto &decl : decls) 
		if (!decl->isVarDecl()) {
			currentScope = globalScope;
			currentClassSymbol = nullptr;
			decl->accept(*this);
		}
}

void GlobalFuncAndClsVisitor::visit(FunctionDecl * node)
{
	std::shared_ptr<SymbolType> retType = symbolTypeOfNode(node->getRetType().get(), globalScope);
	if (retType == nullptr)
		throw SemanticError("undefined type", node->Where());
	
	// it can be a constructor
	bool isConstructor = false;
	if (currentClassSymbol != nullptr 
		&& node->getIdentifier()->name == currentClassSymbol->getSymbolName() + "::ctor") {
		if (currentClassSymbol->getConstructor() != nullptr)
			throw SemanticError("Duplicated constructor.", node->Where());
		isConstructor = true;
		retType = currentClassSymbol;
	}
	auto funcSymbol = std::make_shared<FunctionSymbol>(node->getIdentifier()->name, retType, node, currentScope);
	if (isConstructor)
		currentClassSymbol->setConstructor(funcSymbol);
	node->setFuncSymbol(funcSymbol);
	currentScope->define(funcSymbol);

	currentScope = funcSymbol;
	auto args = node->getArgs();
	for (auto &arg : args) {
		arg->accept(*this);
		arg->markAsArg();
		//funcSymbol->define(arg->getVarSymbol()); 
	}
}

void GlobalFuncAndClsVisitor::visit(ClassDecl * node)
{
	// Class has already been defined
		//maintain pointers
	auto clsSymbol = node->getClsSymbol();
	currentScope = clsSymbol;
	currentClassSymbol = clsSymbol;

	//deal with members
	auto members = node->getMembers();
	for (auto &decl : members){ 
		decl->accept(*this);
		currentScope = currentClassSymbol;
	}
}

void GlobalFuncAndClsVisitor::visit(VarDeclStmt * node)
{
	if (currentClassSymbol != nullptr && node->getInitExpr() != nullptr)
		node->getInitExpr()->accept(*this);
	std::shared_ptr<SymbolType> type = symbolTypeOfNode(node->getType().get(), globalScope);
	if (type == nullptr)
		throw SemanticError("undefined type", node->Where());
	node->setSymbolType(type);
	auto var = std::make_shared<VarSymbol>(node->getIdentifier()->name, type, node);
	node->setVarSymbol(var);
	currentScope->define(var); //need to check whether currentScope == globalScope ?
	std::clog << "define variable " << var->getSymbolName()
		<< " in scopce " << currentScope->getScopeName() << '\n';
}

void GlobalFuncAndClsVisitor::visit(MultiVarDecl * node)
{
	auto vars = node->getDecls();
	for (auto &var : vars) var->accept(*this);
}

void GlobalFuncAndClsVisitor::visit(NewExpr * node)
{
	auto type = symbolTypeOfNode(node->getType().get(), globalScope);
	if (type == nullptr)
		throw SemanticError("undefined type", node->Where());
	node->setSymbolType(type);
	auto dims = node->getDimensions();
	for (auto &dim : dims) dim->accept(*this);
}
