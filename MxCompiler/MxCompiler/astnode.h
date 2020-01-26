#pragma once

#include "pch.h"
#include "position.h"

using NodeId = std::uintptr_t;

class astNode {
public:
	astNode() = default;
	NodeId id() { return (NodeId)this; }
private:
	PosPair pos;
};

class Identifier : public astNode {
public:
	Identifier(const std::string &_name) : name(_name) {}
	std::string name;
};

//Type = BasicType (LeftIndex RightIndex)*
class Type : public astNode {};

class BasicType : public Type {};


// BuiltinType = Int | String | Bool
class BuiltinType : public BasicType{
public:
	enum Typename
	{
		INT, BOOL, STRING
	};
	BuiltinType(Typename _tp) :tp(_tp) {}

private:
	Typename tp;
};

class UserDefinedType : public BasicType {
public:
	UserDefinedType(std::shared_ptr<Identifier> _typeName) :typeName(std::move(_typeName)) {}
private:
	std::shared_ptr<Identifier> typeName;
};

class ArrayType : public Type {
public:
	ArrayType(std::shared_ptr<Type> _typeName) : typeName(_typeName) {}
private:
	std::shared_ptr<Type> typeName;
};

/******************************************Types end here************************************************/

class Expression : public astNode {
};

class IdentifierExpr : public Expression {
public:
	IdentifierExpr(std::shared_ptr<Identifier> _id) :id(std::move(_id)) {}
private:
	std::shared_ptr<Identifier> id;
};

class ConstValue : public Expression{

};

class BoolValue : public ConstValue {
public:
	BoolValue(bool v) : value(v) {}
	bool value;
};

class StringValue : public ConstValue {
public:
	StringValue(const std::string &_str) :str(_str) {}
	std::string str;
};

class NumValue : public ConstValue {
public:
	NumValue(int _num) : num(_num) {}
	int num;
};

class NullValue : public ConstValue {

};

class NewExpr : public Expression {
public:
	NewExpr(std::shared_ptr<Type>_type, std::vector<std::shared_ptr<Expression> > v)
		:type(std::move(_type)),dimensions(std::move(v)){}
private:
	std::shared_ptr<Type> type;
	std::vector<std::shared_ptr<Expression> > dimensions;
};

class UnaryExpr : public Expression {
public:
	enum Operator
	{
		NEG, POS, NOT, INV,
		PREINC, POSTINC,
		PREDEC, POSTDEC
	};
	UnaryExpr(Operator _op, std::shared_ptr<Expression> _oprand) :
		op(_op), oprand(std::move(_oprand)) {}

private:
	Operator op;
	std::shared_ptr<Expression> oprand;
};

class BinaryExpr : public Expression {
public:
	enum  Operator
	{
		ADD, MINUS, TIMES, DIVIDE, MOD,
		LESS, LEQ, GREATER, GEQ, NEQ, EQ,
		LSHIFT,RSHIFT,
		ASSIGN,
		AND, NOT, OR,
		BITAND, BITOR, BITXOR,
		INDEX, MEMBER
	};
	BinaryExpr(Operator _op, std::shared_ptr<Expression> _oprand1, std::shared_ptr<Expression> _oprand2)
		:op(_op), oprand1(std::move(_oprand1)), oprand2(std::move(_oprand2)) {}

private:
	Operator op;
	std::shared_ptr<Expression> oprand1, oprand2;
};

class FuncCallExpr : public Expression {
public:
	FuncCallExpr(std::shared_ptr<Identifier> _funcName, 
		std::vector<std::shared_ptr<Expression> > _args)
		:funcName(std::move(_funcName)), args(std::move(_args)) {}
private:
	std::shared_ptr<Identifier> funcName;
	std::vector<std::shared_ptr<Expression> > args;
};

class MemberFuncCallExpr : public FuncCallExpr {
public:
	MemberFuncCallExpr(std::shared_ptr<Expression> _instance, std::shared_ptr<Identifier> _funcName,
		std::vector<std::shared_ptr<Expression> > _args)
		:FuncCallExpr(std::move(_funcName),std::move(_args)),
		instance(std::move(_instance)){}
private:
	std::shared_ptr<Expression> instance;
};

/******************************************Expressions end here************************************************/

class Statement : public astNode {
};

class VarDeclStmt : public Statement {
public:
	VarDeclStmt(std::shared_ptr<Type> _type,
		std::shared_ptr<Identifier> _id,
		std::shared_ptr<Expression> _init = nullptr) 
	: type(std::move(_type)), id(std::move(_id)), init(std::move(_init)) {}
private:
	std::shared_ptr<Type> type;
	std::shared_ptr<Identifier> id;
	std::shared_ptr<Expression> init;
};

class IfStmt : public Statement {
public:
	IfStmt(std::shared_ptr<Expression> _condition,
		std::shared_ptr<Statement> _then,
		std::shared_ptr<Statement> _ELSE = nullptr)
		:condition(_condition), then(_then), ELSE(_ELSE) {}
private:
	std::shared_ptr<Expression> condition;
	std::shared_ptr<Statement> then;
	std::shared_ptr<Statement> ELSE;
};

class ForStmt : public Statement {
public:
	ForStmt(std::shared_ptr<Statement> _init, std::shared_ptr<Statement> _iter,
		std::shared_ptr<Statement> _body, std::shared_ptr<Expression> _cond)
		:init(std::move(init)), condition(std::move(_cond)), iter(std::move(_iter)),
			body(std::move(_body)) {}
private:
	std::shared_ptr<Statement> init, iter, body;
	std::shared_ptr<Expression> condition;
};

class WhileStmt : public Statement {
public:
	WhileStmt(std::shared_ptr<Expression> _condition,
		std::shared_ptr<Statement> _body) 
		:condition(std::move(_condition)), body(std::move(_body)) {}
private:
	std::shared_ptr<Expression> condition;
	std::shared_ptr<Statement> body;
};

class ExprStmt : public Statement {
public:
	ExprStmt(std::shared_ptr<Expression> _expr) :expr(std::move(_expr)) {};
private:
	std::shared_ptr<Expression> expr;
};

class ReturnStmt : public Statement {
public:
	ReturnStmt(std::shared_ptr<Expression> _value = nullptr) :value(std::move(_value)) {}
private:
	std::shared_ptr<Expression> value; // value == nullptr if return void type.
};

class BreakStmt : public Statement {};

class ContinueStmt : public Statement {};

// EmptyStmt: Semicolon
class EmptyStmt : public Statement {};

class StmtBlock : public Statement {
public:
	StmtBlock(std::vector<std::shared_ptr<Statement> > _stmts)
		:stmts(std::move(_stmts)) {}
private:
	std::vector<std::shared_ptr<Statement> > stmts;
};

/******************************************Statements end here************************************************/

class Declaration : public astNode {};

class GlobalVarDecl : public Declaration {
public:
	GlobalVarDecl(std::shared_ptr<VarDeclStmt> _stmt) : stmt(std::move(stmt)) {}
private:
	std::shared_ptr<VarDeclStmt> stmt;
};

class FunctionDecl : public Declaration {
public:
	FunctionDecl(std::shared_ptr<Type> _retType,
		std::shared_ptr<Identifier> _name,
		std::vector< std::shared_ptr<VarDeclStmt> > _args,
		std::shared_ptr<StmtBlock> _body)
		:retType(std::move(_retType)),name(std::move(_name)), 
		args(std::move(args)), body(std::move(_body)) {}
private:
	//retType == nullptr if this function return with void type.
	std::shared_ptr<Type> retType;
	std::shared_ptr<Identifier> name;
	std::vector< std::shared_ptr<VarDeclStmt> > args;
	std::shared_ptr<StmtBlock> body;
};

class ClassDecl : public Declaration {
public:
	ClassDecl(std::shared_ptr<Identifier> _name,
		std::vector< std::shared_ptr<Declaration> > _members)
		:name(std::move(_name)), members(std::move(_members)) {}
private:
	std::shared_ptr<Identifier> name;
	std::vector< std::shared_ptr<Declaration> > members;
};

/**************************************Program************************************/
class ProgramAST :astNode{
public:
	ProgramAST(std::vector<std::shared_ptr<Declaration> > _decls) 
		:decls(std::move(_decls)) {}
private:
	std::vector<std::shared_ptr<Declaration> > decls;
};