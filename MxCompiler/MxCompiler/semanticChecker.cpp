#include "semanticChecker.h"
#include "astnode.h"
#include "symbol.h"

void SemanticChecker::visit(ProgramAST *node)
{
	auto decls = node->getDecls();
	for (auto &decl : decls) decl->accept(*this);
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
	if (node->getInitExpr() != nullptr) {
		node->getInitExpr()->accept(*this);
		// the initial value must be of the same type.
		if (!node->getSymbolType()->compatible(node->getInitExpr()->getSymbolType()))
			throw SemanticError("incompatible initial value",node->Where());
	}
	else {
		// set the initial value as default(int : 0 , string : "", bool : false).
		auto type = node->getSymbolType();
		if (type->isBulitInType()) {
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
	node->getCondition()->accept(*this);
	if (node->getCondition()->getSymbolType()->getTypeName() != "bool")
		throw SemanticError("condition must be boolean value", node->Where());
	node->getThen()->accept(*this);
	if (node->getElse() != nullptr)
		node->getElse()->accept(*this);
}

void SemanticChecker::visit(WhileStmt * node)
{
	node->getCondition()->accept(*this);
	if (node->getCondition()->getSymbolType()->getTypeName() != "bool")
		throw SemanticError("condition must be boolean value", node->Where());
	node->getBody()->accept(*this);
}

void SemanticChecker::visit(ForStmt * node)
{
	node->getCondition()->accept(*this);
	if (node->getCondition()->getSymbolType()->getTypeName() != "bool")
		throw SemanticError("condition must be boolean value", node->Where());
	node->getInit()->accept(*this);
	node->getIter()->accept(*this);
	node->getBody()->accept(*this);
}

void SemanticChecker::visit(ReturnStmt * node)
{
	// Constructors should not contain return statement.
	if (reinterpret_cast<FunctionDecl *>(node->getFuncSymbol()->getDecl())->isConstructor())
		SemanticError("Constructors should not contain return statement", node->Where());

	auto retType = node->getFuncSymbol()->getType();
	if (node->getValue() == nullptr) { // a void value is returned.
		if (!(retType->getTypeName() != "void"))
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
			(lhsType == "stirng" && rhsType == "string")))
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
			if (lhs->getSymbolType()->isBulitInType() && rhs->getSymbolType()->isBulitInType()) {
				if (lhsType != rhsType)
					throw SemanticError("only same type can be compared", node->Where());
			}
			else throw SemanticError("only bultin types can be compared", node->Where());
		}
		node->setExprCategory(Expression::RVALUE);
		node->setSymbolType(boolSymbol);
	}
	else if (op == BinaryExpr::ASSIGN) {
		if (!(lhs->isLvalue()))
			throw SemanticError("only left value can be assigned", node->Where());
		if (rhs->isObject())
			throw SemanticError("the assigning value cannot be an Object", node->Where());
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
	else {  // interger-only operators :
		// both operands must be int
		if (lhsType != "int" || rhsType != "int")
			throw SemanticError("both operands must be int", node->Where());
		node->setExprCategory(Expression::RVALUE);
		if (isComparisonOperator(op))
			node->setSymbolType(boolSymbol);
		else 
			node->setSymbolType(intSymbol);
	}
}


void SemanticChecker::visit(ClassMemberExpr * node)
{
	auto obj = node->getObj();
	auto varName = node->getIdentifier()->getIdentifier()->name;
	obj->accept(*this);
	if (obj->getExprCategory() == Expression::THIS || obj->isObject()) {
		auto symbol = std::static_pointer_cast<ClassSymbol>(obj->getSymbolType())->resolve(varName);
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
		auto formalArgs = reinterpret_cast<FunctionDecl *>(func->getDecl())->getArgs();
		if (args.size() > formalArgs.size())
			throw SemanticError("too many parameters", node->Where());
		if (args.size() < formalArgs.size())
			throw SemanticError("too few parameters", node->Where());

		for (int i = 0; i < args.size(); i++) {
			if (!args[i]->isValue())
				throw SemanticError("parameter should be a valid value", node->Where());
			if (!formalArgs[i]->getSymbolType()->compatible(args[i]->getSymbolType()))
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

	if (dims.empty()) {  // a single variable
		node->setSymbolType(type);
		if (type->isUserDefinedType()) { // might have constructor
			auto ctor = std::static_pointer_cast<ClassSymbol>(type)->getConstructor();
			if (ctor != nullptr) node->setConstructor(ctor);
		}
	}
	else {  // a new array
		node->setSymbolType(std::make_shared<ArraySymbol>(type));
	}
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
		if (operand->getSymbolType()->getTypeName() == "bool") {
			node->setExprCategory(Expression::RVALUE);
			node->setSymbolType(boolSymbol);
		}
		else throw SemanticError("'~' should be in front of a boolean value",node->Where());
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
	return   op == BinaryExpr::AND || op == BinaryExpr::NOT
		|| op == BinaryExpr::GEQ || op == BinaryExpr::OR;
}

bool SemanticChecker::isComparisonOperator(BinaryExpr::Operator op)
{
	return op == BinaryExpr::LESS || op == BinaryExpr::LEQ
		|| op == BinaryExpr::GREATER || op == BinaryExpr::GEQ;
}










