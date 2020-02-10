#include "GlobalFuncAndClsDecl.h"

void GlobalFuncAndClsVisitor::visit(ProgramAST * node)
{
	auto decls = node->getDecls();
	for (auto decl : decls) 
		if(!decl->isVarDecl()){ // global variables are ignored here
			currentScope = globalScope;
			decl->accept(*this);
		}
}

void GlobalFuncAndClsVisitor::visit(FunctionDecl * node)
{
	std::shared_ptr<SymbolType> retType = symbolTypeOfNode(node->getRetType().get(), globalScope);
	// it can be a constructor
	bool isConstructor = false;
	if (currentClassSymbol != nullptr 
		&& node->getIdentifier()->name == "::ctor") { // constructors
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

	std::clog << "define function: " << funcSymbol->getSymbolName() <<
		" in scope " << currentScope->getScopeName() << '\n';
	
	// Define formal arguments
	currentScope = funcSymbol;
	auto args = node->getArgs();
	for (auto arg : args) arg->accept(*this);
}

void GlobalFuncAndClsVisitor::visit(ClassDecl * node)
{
	//it has been defined in <ClassDeclVisitor>
	auto clsSymbol = node->getClsSymbol();
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

void GlobalFuncAndClsVisitor::visit(VarDeclStmt * node)
{
	if (node->getInitExpr() != nullptr)
		throw SemanticError("illeagal initialization", node->Where());
	std::shared_ptr<SymbolType> type = symbolTypeOfNode(node->getType().get(), globalScope);
	if (type == nullptr)
		throw SemanticError("undefined type", node->Where());
	
	node->setSymbolType(type);
	auto var = std::make_shared<VarSymbol>(node->getIdentifier()->name, type, node);
	node->setVarSymbol(var);
	currentScope->define(var); 
	std::clog << "define variable " << var->getSymbolName()
		<< " in scopce " << currentScope->getScopeName() << '\n';
}
