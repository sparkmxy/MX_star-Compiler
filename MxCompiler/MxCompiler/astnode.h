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
class Type : public astNode {

};

class BasicType : public Type {
public:
	BasicType() = default;
};


// BuiltinType = Int | String | Bool
class BuiltinType : public BasicType{
public:
	BuiltinType() = default;
	enum Typename
	{
		INT, BOOL, STRING
	};
};

class UserDefinedType : public BasicType {
public:
	UserDefinedType(std::shared_ptr<Identifier> _typeName) :typeName(std::move(_typeName)) {}
private:
	std::shared_ptr<Identifier> typeName;
};

class ArrayType : public Type {
public:
	ArrayType(std::shared_ptr<BasicType> _typeName) : typeName(_typeName) {}
private:
	std::shared_ptr<Type> typeName;
};

class Expression : public astNode {
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

};

class UnaryExpr : public Expression {

};

class BinaryExpr : public Expression {
public:
	enum  Operator
	{
		ADD, MINUS, TIMES, DIVIDE, MOD,
		LESS, LEQ, GREATER, GEQ, NEQ, EQ,
		LSHIFT,RSHIFT, ASSIGN,
		AND, NOT, OR,
		BITAND, BITOR, BITXOR
	};
};

class FuncCallExpr : public Expression {

};

/************************statement*************************/


/*
Grammar
program -> module  program | module
moduel -> clsDcl | funcDcl | vardcl

-----------------------------------------------Block
block -> {stmts}

stmts -> stmt stmts | eps

----------------------------------------------Expression
*/
