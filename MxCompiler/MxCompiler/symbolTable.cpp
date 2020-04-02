#include "symbolTable.h"
#include "astnode.h"

void SymbolTable::checkMainFunc()
{
	auto symbol = globalScope->resolve("main");
	if (symbol == nullptr)
		throw SemanticError("missing main function", Position());
	if (symbol->category() != Symbol::FUNCTION)
		throw SemanticError("'main' must be a function", Position());
	auto main = std::static_pointer_cast<FunctionSymbol>(symbol);
	auto retType = main->getType();
	if (retType->isBuiltInType() && retType->getTypeName() == "int") {
		auto args = reinterpret_cast<FunctionDecl *>(main->getDecl())->getArgs();
		if (!args.empty())
			throw SemanticError("'main' shall not have arguments", Position());
	}
	else throw SemanticError("'main' must return int", Position());
}

void SymbolTable::visit(ProgramAST *program) {
	auto decls = program->getDecls();
	for (auto &decl : decls) {
		currentScope = globalScope;
		currentClassSymbol = nullptr;
		currentFunctionSymbol = nullptr;
		decl->accept(*this);
	}
	checkMainFunc();
}


void SymbolTable::visit(VarDeclStmt *node) {
	if (node->getInitExpr() != nullptr)
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

void SymbolTable::visit(ExprStmt * node)
{
	node->getExpr()->accept(*this);
}

void SymbolTable::visit(FuncCallExpr * node)
{
	auto id = node->getIdentifierExpr();
	if(!isConstructor(id->getIdentifier()->name))
		id->accept(*this);
	auto args = node->getArgs();
	for (auto &arg : args) arg->accept(*this);
}

void SymbolTable::visit(ClassMemberExpr * node)
{
	node->getObj()->accept(*this);
	//node->getIdentifier()->accept(*this);
}

void SymbolTable::visit(MemberFuncCallExpr * node)
{
	node->getInstance()->accept(*this);
	//auto cls = std::static_pointer_cast<ClassSymbol>(node->getInstance()->getSymbolType());
	auto args = node->getArgs();
	for (auto &arg : args) arg->accept(*this);
}

void SymbolTable::visit(IdentifierExpr * node)
{
	if (node->getIdentifier()->name == "this") {
		if (currentClassSymbol == nullptr)
			throw SemanticError("'this' can only be used in class definition", node->Where());
		node->setSymbol(currentClassSymbol);
		node->setExprCategory(Expression::THIS);
	}
	else {
		auto symbol = currentScope->resolve(node->getIdentifier()->name);
		if (symbol == nullptr)
			throw SemanticError("undefined identifier", node->Where());
		node->setSymbol(symbol);
	}
}

void SymbolTable::visit(NewExpr * node)
{
	auto type = symbolTypeOfNode(node->getType().get(), globalScope);
	if (type == nullptr)
		throw SemanticError("undefined type", node->Where());
	node->setSymbolType(type);
	auto dims = node->getDimensions();
	for (auto &dim : dims) dim->accept(*this);

	if (type->isUserDefinedType()) { // might have constructor
		auto ctor = std::static_pointer_cast<ClassSymbol>(type)->getConstructor();
		if (ctor != nullptr) {
			node->setConstructor(ctor);
		}
		if (node->getCtorCall() != nullptr) {
			node->getCtorCall()->getIdentifierExpr()->setSymbol(ctor);
			node->getCtorCall()->accept(*this);
		}
	}
}

void SymbolTable::visit(UnaryExpr * node)
{
	node->getOperand()->accept(*this);
}

void SymbolTable::visit(BinaryExpr * node)
{
	node->getLHS()->accept(*this);
	node->getRHS()->accept(*this);
}

void SymbolTable::visit(ThisExpr * node)
{
	if (currentClassSymbol == nullptr)
		throw SemanticError("'this' must be used in class declaration", node->Where());
	node->setExprCategory(Expression::THIS);
	node->setSymbolType(currentClassSymbol);
}


void SymbolTable::visit(MultiVarDecl * node)
{
	auto vars = node->getDecls();
	for (auto &var : vars) var->accept(*this);
}

/*
Function symbol and formal argumnets are already defined in <GlobalFuncAndClsVistor>
*/
void SymbolTable::visit(FunctionDecl *node) {

	auto funcSymbol = node->getFuncSymbol();
	currentScope = funcSymbol;
	currentFunctionSymbol = funcSymbol;
	node->getBody()->accept(*this);

	if (currentClassSymbol != nullptr &&
		node->getIdentifier()->name == currentClassSymbol->getSymbolName())
		throw SemanticError("duplicated identifier.", node->Where());
}

void SymbolTable::visit(ClassDecl *node)
{
	//maintain pointers
	auto clsSymbol = node->getClsSymbol();
	currentScope = clsSymbol;
	currentClassSymbol = clsSymbol;
	//deal with members
	auto members = node->getMembers();
	for (auto &decl : members) 
		if(!decl->isVarDecl()){   // variable members has been defined in <GlobalFuncAndClsVisitor>
			decl->accept(*this);
			currentScope = clsSymbol;
		}
}

void SymbolTable::visit(StmtBlock * node)
{
	auto local = std::make_shared<LocalScope>(currentScope->getScopeName() + "_LOCAL", currentScope);
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
	if (node->getInit() != nullptr) node->getInit()->accept(*this);
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
		throw SemanticError("'break' must be in a loop.", node->Where());
	node->setLoop(loops.top());
}


void SymbolTable::visit(ContinueStmt * node)
{
	if(loops.empty())
		throw SemanticError("'continue' must be in a loop.", node->Where());
	node->setLoop(loops.top());
}

