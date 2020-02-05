#pragma once
/*
This file implements the <astNode> class family.
*/
#include "pch.h"
#include "position.h"
#include "symbol.h"
#include "visitor.h"

#define ACCEPT_VISITOR void accept (Visitor &vis) override {vis.visit(this);} 
#define ACCEPT_VISITOR_VIRTUAL void accept (Visitor &vis) override = 0;

using NodeId = std::uintptr_t;

class astNode{
public:
	astNode() = default;
	NodeId id() { return (NodeId)this; }
	virtual void accept(Visitor &vis) = 0;

	Position Where() { return pos.first; }
private:
	PosPair pos;
};

class Identifier : public astNode{
public:
	Identifier(const std::string &_name) : name(_name) {}
	ACCEPT_VISITOR

	std::string name;
};

//Type = BasicType (LeftIndex RightIndex)*
class Type : public astNode{
public:
	ACCEPT_VISITOR_VIRTUAL
	virtual bool isArrayType() = 0;
	virtual std::string getIdentifier() = 0;
};

class BasicType : public Type{
public:
	ACCEPT_VISITOR_VIRTUAL
	bool isArrayType() override { return false; }
	virtual std::string getIdentifier() = 0;
};


// BuiltinType = Int | String | Bool
class BuiltinType : public BasicType{
public:
	enum Typename
	{
		INT, BOOL, STRING
	};
	BuiltinType(Typename _tp) :tp(_tp) {}
	std::string getIdentifier() override{
		if (tp == INT) return "int";
		if (tp == BOOL) return "bool";
		if (tp == STRING) return "string";
	}
	ACCEPT_VISITOR
private:
	Typename tp;
};

class UserDefinedType : public BasicType{
public:
	UserDefinedType(std::shared_ptr<Identifier> _typeName) :identifier(std::move(_typeName)) {}
	std::string getIdentifier() override { return identifier->name; }

	ACCEPT_VISITOR
private:
	std::shared_ptr<Identifier> identifier;
};

class ArrayType : public Type{
public:
	ArrayType(std::shared_ptr<Type> _typeName) : baseTypeName(_typeName) {}
	bool isArrayType() override { return true; }
	std::string getIdentifier() override { return baseTypeName->getIdentifier(); }

	ACCEPT_VISITOR
private:
	std::shared_ptr<Type> baseTypeName;
};

/******************************************Types end here************************************************/

class Expression : public astNode{
public:
	ACCEPT_VISITOR_VIRTUAL
};

class IdentifierExpr : public Expression{
public:
	IdentifierExpr(std::shared_ptr<Identifier> _id) :id(std::move(_id)) {}
	ACCEPT_VISITOR
private:
	std::shared_ptr<Identifier> id;
};

class ConstValue : public Expression, public std::enable_shared_from_this<ConstValue> {
public:
	ACCEPT_VISITOR_VIRTUAL
};

class BoolValue : public ConstValue, public std::enable_shared_from_this<BoolValue> {
public:
	BoolValue(bool v) : value(v) {}
	ACCEPT_VISITOR
private:
	bool value;
};

class StringValue : public ConstValue, public  std::enable_shared_from_this<StringValue> {
public:
	StringValue(const std::string &_str) :str(_str) {}
	ACCEPT_VISITOR
private:
	std::string str;
};

class NumValue : public ConstValue{
public:
	NumValue(int _num) : num(_num) {}
	ACCEPT_VISITOR
private:
	int num;
};

class NullValue : public ConstValue{
public:
	ACCEPT_VISITOR_VIRTUAL
};

class NewExpr : public Expression {
public:
	NewExpr(std::shared_ptr<Type>_type, std::vector<std::shared_ptr<Expression> > v)
		:type(std::move(_type)),dimensions(std::move(v)){}
	ACCEPT_VISITOR
private:
	std::shared_ptr<Type> type;
	std::vector<std::shared_ptr<Expression> > dimensions;
};

class UnaryExpr : public Expression{
public:
	enum Operator
	{
		NEG, POS, NOT, INV,
		PREINC, POSTINC,
		PREDEC, POSTDEC
	};
	UnaryExpr(Operator _op, std::shared_ptr<Expression> _oprand) :
		op(_op), oprand(std::move(_oprand)) {}

	ACCEPT_VISITOR
private:
	Operator op;
	std::shared_ptr<Expression> oprand;
};

class BinaryExpr : public Expression{
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

	ACCEPT_VISITOR
private:
	Operator op;
	std::shared_ptr<Expression> oprand1, oprand2;
};

class FuncCallExpr : public Expression{
public:
	FuncCallExpr(std::shared_ptr<Identifier> _funcName, 
		std::vector<std::shared_ptr<Expression> > _args)
		:funcName(std::move(_funcName)), args(std::move(_args)) {}

	ACCEPT_VISITOR
private:
	std::shared_ptr<Identifier> funcName;
	std::vector<std::shared_ptr<Expression> > args;
};

class MemberFuncCallExpr : public Expression{
public:
	MemberFuncCallExpr(std::shared_ptr<Expression> _instance, std::shared_ptr<Identifier> _funcName,
		std::vector<std::shared_ptr<Expression> > _args)
		:funcName(std::move(_funcName)),args(std::move(_args)),
		instance(std::move(_instance)){}

	ACCEPT_VISITOR
private:
	std::shared_ptr<Identifier> funcName;
	std::vector<std::shared_ptr<Expression> > args;
	std::shared_ptr<Expression> instance;
};

/******************************************Expressions end here************************************************/

class Statement : public astNode{
public:
	ACCEPT_VISITOR_VIRTUAL
};

class VarDeclStmt : public Statement{
public:
	VarDeclStmt(std::shared_ptr<Type> _type,
		std::shared_ptr<Identifier> _id,
		std::shared_ptr<Expression> _init = nullptr) 
	: type(std::move(_type)), identifier(std::move(_id)), init(std::move(_init)) {}
	ACCEPT_VISITOR

	std::shared_ptr<Expression> getInitExpr() { return init; }
	std::shared_ptr<Type> getType() { return type; }
	std::shared_ptr<Identifier> getIdentifier() { return identifier; }

	void setVarSymbol(std::shared_ptr<VarSymbol> _varSymbol) { varSymbol = _varSymbol; }
private:
	std::shared_ptr<Type> type;
	std::shared_ptr<Identifier> identifier;
	std::shared_ptr<Expression> init;
	
	std::shared_ptr<VarSymbol> varSymbol;
};

class IfStmt : public Statement {
public:
	IfStmt(std::shared_ptr<Expression> _condition,
		std::shared_ptr<Statement> _then,
		std::shared_ptr<Statement> _ELSE = nullptr)
		:condition(_condition), then(_then), ELSE(_ELSE) {}
	ACCEPT_VISITOR
private:
	std::shared_ptr<Expression> condition;
	std::shared_ptr<Statement> then;
	std::shared_ptr<Statement> ELSE;
};

class ForStmt : public Statement{
public:
	ForStmt(std::shared_ptr<Statement> _init, std::shared_ptr<Statement> _iter,
		std::shared_ptr<Statement> _body, std::shared_ptr<Expression> _cond)
		:init(std::move(init)), condition(std::move(_cond)), iter(std::move(_iter)),
			body(std::move(_body)) {}
	ACCEPT_VISITOR
private:
	std::shared_ptr<Statement> init, iter, body;
	std::shared_ptr<Expression> condition;
};

class WhileStmt : public Statement {
public:
	WhileStmt(std::shared_ptr<Expression> _condition,
		std::shared_ptr<Statement> _body) 
		:condition(std::move(_condition)), body(std::move(_body)) {}
	ACCEPT_VISITOR
private:
	std::shared_ptr<Expression> condition;
	std::shared_ptr<Statement> body;
};

class ExprStmt : public Statement{
public:
	ExprStmt(std::shared_ptr<Expression> _expr) :expr(std::move(_expr)) {};
	ACCEPT_VISITOR
private:
	std::shared_ptr<Expression> expr;
};

class ReturnStmt : public Statement{
public:
	ReturnStmt(std::shared_ptr<Expression> _value = nullptr) :value(std::move(_value)) {}
	ACCEPT_VISITOR
private:
	std::shared_ptr<Expression> value; // value == nullptr if return void type.
};

class BreakStmt : public Statement{
public:
	ACCEPT_VISITOR
};

class ContinueStmt : public Statement{
public:
	ACCEPT_VISITOR
};

// EmptyStmt: Semicolon
class EmptyStmt : public Statement{
public:
	ACCEPT_VISITOR
};

class StmtBlock : public Statement{
public:
	StmtBlock(std::vector<std::shared_ptr<Statement> > _stmts)
		:stmts(std::move(_stmts)) {}

	ACCEPT_VISITOR
private:
	std::vector<std::shared_ptr<Statement> > stmts;
};

/******************************************Statements end here************************************************/

class Declaration : public astNode{
public:
	ACCEPT_VISITOR_VIRTUAL
};

class VarDecl : public Declaration{
public:
	VarDecl(std::shared_ptr<VarDeclStmt> _stmt) : stmt(std::move(stmt)) {}

	std::shared_ptr<VarDeclStmt> getStmt() { return stmt; }
	void setSymbolType(std::shared_ptr<SymbolType> _type) { typeOfSymbol = _type; }

	ACCEPT_VISITOR
private:
	std::shared_ptr<VarDeclStmt> stmt;
	std::shared_ptr<SymbolType> typeOfSymbol;
};

class FunctionDecl : public Declaration{
public:
	FunctionDecl(std::shared_ptr<Type> _retType,
		std::shared_ptr<Identifier> _name,
		std::vector< std::shared_ptr<VarDeclStmt> > _args,
		std::shared_ptr<StmtBlock> _body)
		:retType(std::move(_retType)),name(std::move(_name)), 
		args(std::move(args)), body(std::move(_body)) {}
	
	/*getters/setters*/
	std::shared_ptr<Type> getRetType() { return retType; }
	std::shared_ptr<Identifier> getIdentifier() { return name; }
	std::vector< std::shared_ptr<VarDeclStmt> > getArgs() { return args; }
	std::shared_ptr<StmtBlock> getBody() { return body; }

	void setFuncSymbol(std::shared_ptr<FunctionSymbol> _funcSymbol) { funcSymbol = _funcSymbol; }
	
	ACCEPT_VISITOR
private:
	//retType == nullptr if this function return with void type.
	std::shared_ptr<Type> retType;
	std::shared_ptr<Identifier> name;
	std::vector< std::shared_ptr<VarDeclStmt> > args;
	std::shared_ptr<StmtBlock> body;

	std::shared_ptr<FunctionSymbol> funcSymbol;
};

class ClassDecl : public Declaration{
public:
	ClassDecl(std::shared_ptr<Identifier> _name,
		std::vector< std::shared_ptr<Declaration> > _members)
		:name(std::move(_name)), members(std::move(_members)) {}
	
	std::shared_ptr<Identifier> getIdentifier() { return name; }

	void setClsSymbol(std::shared_ptr<ClassSymbol> _clsSymbol) { clsSymbol = _clsSymbol; }

	ACCEPT_VISITOR
private:
	std::shared_ptr<Identifier> name;
	std::vector< std::shared_ptr<Declaration> > members;
	
	std::shared_ptr<ClassSymbol> clsSymbol;
};

/**************************************Program************************************/
class ProgramAST :public astNode{
public:
	ProgramAST(std::vector<std::shared_ptr<Declaration> > _decls) 
		:decls(std::move(_decls)) {}
	ACCEPT_VISITOR

	std::vector<std::shared_ptr<Declaration> > &getDecls(){ return decls; }
private:
	std::vector<std::shared_ptr<Declaration> > decls;
};