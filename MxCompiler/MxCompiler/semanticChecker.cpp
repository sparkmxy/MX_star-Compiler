#include "semanticChecker.h"

void SemanticChecker::visit(ProgramAST *node)
{
	auto decls = node->getDecls();

	isScanGlobalVar = true;
	for (auto &decl : decls) 
		if(decl->isVarDecl())decl->accept(*this);
	isScanGlobalVar = false;
	for (auto &decl : decls)
		if (!decl->isVarDecl())decl->accept(*this);
}

void SemanticChecker::visit(MultiVarDecl * node)
{
	auto vars = node->getDecls();
	for (auto &var : vars) var->accept(*this);
}

void SemanticChecker::visit(FunctionDecl * node)
{
	node->getBody()->accept(*this);
}

void SemanticChecker::visit(ClassDecl * node)
{
	auto members = node->getMembers();
	for (auto &member : members) member->accept(*this);
}

void SemanticChecker::visit(VarDeclStmt * node)
{
	if (!isScanGlobalVar && node->getInitExpr() != nullptr) {
		node->getInitExpr()->accept(*this);
		// the initial value must be of the same type.
		if (!node->getSymbolType()->compatible(node->getInitExpr()->getSymbolType()))
			throw SemanticError("incompatible initial value",node->Where());
	}
	else {
		// set the initial value as default(int : 0 , string : "", bool : false).
		auto type = node->getSymbolType();
		if (type->isBuiltInType()) {
			if (type->getTypeName() == "int")
				node->setInitExpr(std::make_shared<NumValue>(0));
			else
				node->setInitExpr(std::make_shared<BoolValue>(false));
		}
		else {
			// Array,String , or User Defined Type
			if (type->getTypeName() == "string")
				node->setInitExpr(std::make_shared<StringValue>("nmsl"));
			else
				node->setInitExpr(std::make_shared<NullValue>());
		}
		node->getInitExpr()->accept(*this);
	}

	if (node->getIdentifier()->name == node->getSymbolType()->getTypeName())
		throw SemanticError("duplicated identifier.", node->Where());
}

void SemanticChecker::visit(StmtBlock * node)
{
	auto stmts = node->getStmts();
	for (auto &stmt : stmts) stmt->accept(*this);
}

void SemanticChecker::visit(ExprStmt * node)
{
	node->getExpr()->accept(*this);
}

void SemanticChecker::visit(IfStmt * node)
{
	auto cond = node->getCondition();
	cond->accept(*this);
	if (cond->getSymbolType()->getTypeName() != "bool")
		throw SemanticError("condition must be boolean value", node->Where());
	node->getThen()->accept(*this);
	if (node->getElse() != nullptr)
		node->getElse()->accept(*this);
}

void SemanticChecker::visit(WhileStmt * node)
{
	auto cond = node->getCondition();
	cond->accept(*this);
	if (cond->getSymbolType()->getTypeName() != "bool")
		throw SemanticError("condition must be boolean value", node->Where());
	node->getBody()->accept(*this);
}

void SemanticChecker::visit(ForStmt * node)
{
	auto condition = node->getCondition();
	if (condition != nullptr) {
		node->getCondition()->accept(*this);
		if (node->getCondition()->getSymbolType()->getTypeName() != "bool")
			throw SemanticError("condition must be boolean value", node->Where());
	}
	node->getInit()->accept(*this);
	node->getIter()->accept(*this);
	node->getBody()->accept(*this);
}

void SemanticChecker::visit(ReturnStmt * node)
{
	// Constructors should not contain return statement.
	if (reinterpret_cast<FunctionDecl *>(node->getFuncSymbol()->getDecl())->isConstructor()) {
		if(node->getValue() != nullptr)
			throw SemanticError("Constructors should return void", node->Where());
		else return;
	}

	auto retType = node->getFuncSymbol()->getType();
	if (node->getValue() == nullptr) { // a void value is returned.
		if (retType->getTypeName() != "void")
			throw SemanticError("return value is expected", node->Where());
	}
	else {
		node->getValue()->accept(*this);
		if (!retType->compatible(node->getValue()->getSymbolType()))
			throw SemanticError("return with a wrong value type", node->Where());
	}
}

void SemanticChecker::visit(BinaryExpr * node)
{
	auto lhs = node->getLHS();
	auto rhs = node->getRHS();
	auto op = node->getOperator();
	lhs->accept(*this);
	rhs->accept(*this);
	auto lhsType = lhs->getSymbolType()->getTypeName();
	auto rhsType = rhs->getSymbolType()->getTypeName();
	if (isBoolOnlyOperator(op)) {
		// both operands must be boolean
		if (lhsType != "bool" || rhsType != "bool")
			throw SemanticError("both operands must be boolean", node->Where());
		node->setExprCategory(Expression::RVALUE);
		node->setSymbolType(boolSymbol);
	}
	else if (op == BinaryExpr::ADD) {
		// <int + int> or <string + string> is requied
		if (!((lhsType == "int" && rhsType == "int") ||
			(lhsType == "string" && rhsType == "string")))
			throw SemanticError("<int + int> or <string + string> is requied", node->Where());
		node->setExprCategory(Expression::RVALUE);
		if(lhsType == "int") node->setSymbolType(intSymbol);
		else node->setSymbolType(stringClassSymbol);
	}
	else if (op == BinaryExpr::EQ || op == BinaryExpr::NEQ) {
		// note that (array,object) == null is leagal
		if (lhs->isNull()) {
			if (!(rhs->nullable()))
				throw SemanticError("incorrect usage of null", node->Where());
		}
		else if (rhs->isNull()) {
			if (!(lhs->nullable())) 
				throw SemanticError("incorrect usage of null", node->Where());
		}
		else {  // both are not null
			if (lhs->getSymbolType()->isBuiltInType() && rhs->getSymbolType()->isBuiltInType()) {
				if (lhsType != rhsType)
					throw SemanticError("only same type can be compared", node->Where());
			}
			else throw SemanticError("only bultin types can be compared", node->Where());
		}
		node->setExprCategory(Expression::RVALUE);
		node->setSymbolType(boolSymbol);
	}
	else if (op == BinaryExpr::ASSIGN) { 
		//note that rhs could be null value
	
		if (!(lhs->isLvalue()))
			throw SemanticError("only left value can be assigned", node->Where());
		if (!(lhs->getSymbolType()->compatible(rhs->getSymbolType())))
			throw SemanticError("assigning between incompatible types",node->Where());
		
		node->setSymbolType(voidSymbol);
		node->setExprCategory(Expression::RVALUE);
	}
	else if (op == BinaryExpr::INDEX) {
		if (!(rhsType == "int"))
			throw SemanticError("index must be integer", node->Where());
		if (!(lhs->getSymbolType()->isArrayType()))
			throw SemanticError("index should be after array identifier", node->Where());
		node->setExprCategory(Expression::LVALUE);
		node->setSymbolType(std::static_pointer_cast<ArraySymbol>(lhs->getSymbolType())->getBaseType());
	}
	else if (isComparisonOperator(op)) { // string op string or int op int
		node->setExprCategory(Expression::RVALUE);
		node->setSymbolType(boolSymbol);
		if (!((lhsType == "int" && rhsType == "int") || (lhsType == "string" && rhsType == "string"))) 
			throw SemanticError("oprand type does not fit the operator.", node->Where());
	}
	else {  // interger-only operators :
		// both operands must be int
		if (lhsType != "int" || rhsType != "int")
			throw SemanticError("both operands must be int", node->Where());
		node->setExprCategory(Expression::RVALUE);
		node->setSymbolType(intSymbol);
	}
}


void SemanticChecker::visit(ClassMemberExpr * node)
{
	auto obj = node->getObj();
	auto varName = node->getIdentifier()->getIdentifier()->name;
	obj->accept(*this);
	if (obj->getExprCategory() == Expression::THIS || obj->isObject()) {
		auto cls = std::static_pointer_cast<ClassSymbol>(obj->getSymbolType());
		auto symbol = cls->resolve(varName);
		//symbol->setScope(cls); ??
		if (symbol == nullptr || symbol->category() != Symbol::VAR)
			throw SemanticError("variable is undefined in class", node->Where());

		node->setSymbol(std::static_pointer_cast<VarSymbol>(symbol));
		node->setSymbolType(symbol->getType());
		node->setExprCategory(Expression::LVALUE);
	}
	else throw SemanticError("'.' must be after an object type", node->Where());
}

void SemanticChecker::visit(MemberFuncCallExpr * node)
{
	auto obj = node->getInstance();
	auto funcName = node->getIdentifierExpr()->getIdentifier()->name;
	obj->accept(*this);
	if (obj->getExprCategory() == Expression::THIS || obj->isObject()) {
		auto symbol = std::static_pointer_cast<ClassSymbol>(obj->getSymbolType())->resolve(funcName);
		if (symbol == nullptr || symbol->category() != Symbol::FUNCTION)
			throw SemanticError("member function is undefined in class", node->Where());
		auto func = std::static_pointer_cast<FunctionSymbol>(symbol);
		node->setFuncSymbol(func);
		
		auto args = node->getArgs();

		for (auto &arg : args)
			arg->accept(*this);
		auto formalArgs = func->getFormalArgs();
		if (args.size() > formalArgs.size())
			throw SemanticError("too many parameters", node->Where());
		if (args.size() < formalArgs.size())
			throw SemanticError("too few parameters", node->Where());
		for (int i = 0; i < args.size(); i++) {
			if (!args[i]->isValue())
				throw SemanticError("parameter should be a valid value", node->Where());
			if (!formalArgs[i]->getType()->compatible(args[i]->getSymbolType()))
				throw SemanticError("wrong parameter type", node->Where());
		}

		node->setSymbolType(func->getType());
		node->setExprCategory(Expression::RVALUE);
	}
	else if (obj->getSymbolType()->isArrayType() && funcName == "size") {
		//array.size()
		if(node->getArgs().size() != 0)
			throw SemanticError("too many parameters", node->Where());
		node->setFuncSymbol(globalScope->getArraySizeFuncSymbol());
		node->setExprCategory(Expression::RVALUE);
		node->setSymbolType(intSymbol);
	}
	else throw SemanticError("'.' must be after an object type", node->Where());
}

void SemanticChecker::visit(FuncCallExpr * node)
{
	// this is only for global function call
	node->getIdentifierExpr()->accept(*this);
	auto args = node->getArgs();
	auto func = std::static_pointer_cast<FunctionSymbol>(
		node->getIdentifierExpr()->getSymbol());
	node->setFuncSymbol(func);
	for (auto &arg : args) arg->accept(*this);
	auto formalArgs = func->getFormalArgs();
	if (args.size() > formalArgs.size())
		throw SemanticError("too many parameters", node->Where());
	if (args.size() < formalArgs.size())
		throw SemanticError("too few parameters", node->Where());

	for (int i = 0; i < args.size(); i++) {
		if (!args[i]->isValue())
			throw SemanticError("parameter should be a valid value", node->Where());
		if (!formalArgs[i]->getType()->compatible(args[i]->getSymbolType()))
			throw SemanticError("wrong parameter type", node->Where());
	}
	node->setSymbolType(func->getType());
	node->setExprCategory(Expression::RVALUE);
}

void SemanticChecker::visit(IdentifierExpr * node)
{
	if (node->getIdentifier()->name == "this") return;
	auto symbol = node->getSymbol();
	node->setSymbolType(symbol->getType());
	if (symbol->getSymbolName() == symbol->getType()->getTypeName())
		throw SemanticError("identifier is a typename.",node->Where());
	if (symbol->category() == Symbol::VAR)
		node->setExprCategory(Expression::LVALUE);
	else if (symbol->category() == Symbol::CLASS)
		node->setExprCategory(Expression::CLASS);
	else if (symbol->category() == Symbol::FUNCTION)
		node->setExprCategory(Expression::FUNCTION); // is this necessary?
}

void SemanticChecker::visit(NewExpr * node)
{
	auto dims = node->getDimensions();
	for (auto &dim : dims) {
		dim->accept(*this);
		if (dim->getSymbolType()->getTypeName() != "int")
			throw SemanticError("array must have integer dimension", node->Where());
	}
	node->setExprCategory(Expression::RVALUE);

	auto type = node->getSymbolType();

	if (node->getConstructor() != nullptr && node->getCtorCall() != nullptr)
		node->getCtorCall()->accept(*this);
}

void SemanticChecker::visit(UnaryExpr * node)
{
	node->getOperand()->accept(*this);
	auto op = node->getOperator();
	auto operand = node->getOperand();
	if (op == UnaryExpr::PREDEC || op == UnaryExpr::PREINC) {
		if (operand->isLvalue() && operand->getSymbolType()->getTypeName() == "int") {
			node->setExprCategory(Expression::LVALUE);
			node->setSymbolType(intSymbol);
		}
		else throw SemanticError("prefix dec/inc require int oprand", node->Where());
	}
	else if (op == UnaryExpr::POSTINC || op == UnaryExpr::POSTDEC) {
		if (operand->isLvalue() && operand->getSymbolType()->getTypeName() == "int") {
			node->setExprCategory(Expression::RVALUE);
			node->setSymbolType(intSymbol);
		}
		else throw SemanticError("post dec/inc require int oprand", node->Where());
	}
	else if (op == UnaryExpr::NOT) {
		auto TypeName = operand->getSymbolType()->getTypeName();
		if (TypeName == "bool" || TypeName == "int") { // int value works too
			node->setExprCategory(Expression::RVALUE);
			node->setSymbolType(boolSymbol);
		}
		else throw SemanticError("'!' should be in front of a boolean value",node->Where());
	}
	else { // + - or ~
		if (operand->getSymbolType()->getTypeName() == "int") {
			node->setExprCategory(Expression::RVALUE);
			node->setSymbolType(intSymbol);
		}
		else throw SemanticError("post dec/inc require int value", node->Where());
	}
}

void SemanticChecker::visit(StringValue * node)
{
	node->setExprCategory(Expression::RVALUE);
	node->setSymbolType(stringClassSymbol);
}

void SemanticChecker::visit(NumValue * node)
{
	node->setExprCategory(Expression::RVALUE);
	node->setSymbolType(intSymbol);
}

void SemanticChecker::visit(BoolValue * node)
{
	node->setExprCategory(Expression::RVALUE);
	node->setSymbolType(boolSymbol);
}

void SemanticChecker::visit(NullValue * node)
{
	node->setExprCategory(Expression::RVALUE);
	node->setSymbolType(std::make_shared<NullTypeSymbol>());
}

bool SemanticChecker::isBoolOnlyOperator(BinaryExpr::Operator op)
{
	return op == BinaryExpr::AND || op == BinaryExpr::OR;
}

bool SemanticChecker::isComparisonOperator(BinaryExpr::Operator op)
{
	return op == BinaryExpr::LESS || op == BinaryExpr::LEQ
		|| op == BinaryExpr::GREATER || op == BinaryExpr::GEQ;
}









