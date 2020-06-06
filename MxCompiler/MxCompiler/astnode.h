#pragma once
/*
This file implements the <astNode> class family.
Much information is attached in later stage.
*/
#include "pch.h"
#include "position.h"
#include "visitor.h"
#include "symbol.h"

#define ACCEPT_VISITOR void accept (Visitor &vis) override {vis.visit(this);} 
#define ACCEPT_VISITOR_VIRTUAL void accept (Visitor &vis) override = 0;

class BasicBlock;
class Operand;

class astNode{
public:
	astNode() = default;
	virtual void accept(Visitor &vis) = 0;

	Position Where() { return pos.first; }
	Position endPos() { return pos.second; }
	void setPos(const PosPair &_pos) { pos = _pos; }
	PosPair getPos() { return pos; }
private:
	PosPair pos;
};

class Identifier :public virtual  astNode{
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
	std::string getIdentifier() override { return baseTypeName->getIdentifier() + "_array"; }
	std::shared_ptr<Type>  getBaseType() { return baseTypeName; }
	ACCEPT_VISITOR
private:
	std::shared_ptr<Type> baseTypeName;
};

/******************************************Types end here************************************************/

class Expression : public virtual astNode {
public:
	enum ExprCategory
	{
		UNDETERMINED, 
		LVALUE, RVALUE, CLASS, FUNCTION, THIS
	};

	Expression(){}
	//Semantic information
	void setSymbolType(std::shared_ptr<SymbolType> _symbolType) { symbolType = _symbolType; }
	std::shared_ptr<SymbolType> getSymbolType() { return symbolType; }

	ExprCategory getExprCategory() { return category; }
	void setExprCategory(ExprCategory _c) { category = _c; }

	bool isLvalue() { return category == LVALUE; }
	bool isValue() { return category == LVALUE || category == RVALUE || category == THIS; }
	bool isNull() { return symbolType->isNull(); }
	bool nullable() {
		if (isNull()) return true;
		if (category != LVALUE) return false;
		return symbolType->isArrayType() || symbolType->isUserDefinedType();
	}
	bool isObject() { return isValue() && symbolType->isUserDefinedType(); }

	// For IR 
	//In IR stage, bool expression require extra information.
	bool isBool() { return isValue() && symbolType->getTypeName() == "bool"; }
	bool isString() { return isValue() && symbolType->getTypeName() == "string"; }
	std::shared_ptr<BasicBlock> getTrueBlock() { return trueBlock; }
	void setTrueBlock(std::shared_ptr<BasicBlock> block) { trueBlock = block; }

	std::shared_ptr<BasicBlock> getFalseBlock() { return falseBlock; }
	void setFalseBlock(std::shared_ptr<BasicBlock> block) { falseBlock = block; }

	std::shared_ptr<Operand> getResultOprand() { return result; }
	void setResultOprand(const std::shared_ptr<Operand> &op) { result = op; }
	
	bool isControl() { return trueBlock != nullptr; }
	ACCEPT_VISITOR_VIRTUAL
private:
	//semantic
	ExprCategory category;
	std::shared_ptr<SymbolType> symbolType;

	//IR 
	std::shared_ptr<BasicBlock> trueBlock, falseBlock;
	std::shared_ptr<Operand> result;
};

class IdentifierExpr : public Expression{
public:
	IdentifierExpr(std::shared_ptr<Identifier> _id) :id(std::move(_id)) {}
	
	std::shared_ptr<Identifier> getIdentifier() { return id;}
	std::shared_ptr<Symbol> getSymbol() { return symbol; }

	void setSymbol(std::shared_ptr<Symbol> _symbol) { symbol = _symbol; }
	
	ACCEPT_VISITOR
private:
	std::shared_ptr<Identifier> id;
	std::shared_ptr<Symbol> symbol;
};

class ConstValue : public Expression{
public:
	ACCEPT_VISITOR_VIRTUAL
};

class BoolValue : public ConstValue {
public:
	BoolValue(bool v) : value(v) {}
	bool getValue() { return value; }
	ACCEPT_VISITOR
private:
	bool value;
};

class StringValue : public ConstValue{
public:
	StringValue(const std::string &_str) :str(_str) {}
	std::string getText() { return str; }
	ACCEPT_VISITOR
private:
	std::string str;
};

class NumValue : public ConstValue{
public:
	NumValue(int _num) : num(_num) {}
	int getValue() { return num; }
	ACCEPT_VISITOR
private:
	int num;
};

class NullValue : public ConstValue{
public:
	ACCEPT_VISITOR
};

class UnaryExpr : public Expression{
public:
	enum Operator
	{
		NEG, POS, NOT, INV,
		PREINC, PREDEC,
		POSTINC, POSTDEC
	};
	UnaryExpr(Operator _op, std::shared_ptr<Expression> _oprand) :
		op(_op), oprand(std::move(_oprand)) {}

	std::shared_ptr<Expression> getOperand() { return oprand; }
	Operator getOperator() { return op; }
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
		AND,OR,
		BITAND, BITOR, BITXOR,
		INDEX
	};

	BinaryExpr(Operator _op, std::shared_ptr<Expression> _oprand1, std::shared_ptr<Expression> _oprand2)
		:op(_op), oprand1(std::move(_oprand1)), oprand2(std::move(_oprand2)) {}

	std::shared_ptr<Expression> getLHS() { return oprand1; }
	std::shared_ptr<Expression> getRHS() { return oprand2; }
	Operator getOperator() { return op; }
	ACCEPT_VISITOR
private:
	Operator op;
	std::shared_ptr<Expression> oprand1, oprand2;
};

class FuncCallExpr : public Expression {
public:
	FuncCallExpr(std::shared_ptr<IdentifierExpr> _funcName,
		std::vector<std::shared_ptr<Expression> > _args)
		:funcName(std::move(_funcName)), args(std::move(_args)) {}

	std::vector<std::shared_ptr<Expression> > &getArgs() { return args; }
	std::shared_ptr<IdentifierExpr> getIdentifierExpr() { return funcName; }
	std::shared_ptr<FunctionSymbol> getFuncSymbol() { return funcSymbol; }

	void setFuncSymbol(std::shared_ptr<FunctionSymbol> _funcSymbol) { funcSymbol = _funcSymbol; }
	
	ACCEPT_VISITOR
private:
	std::shared_ptr<IdentifierExpr> funcName;
	std::vector<std::shared_ptr<Expression> > args;

	std::shared_ptr<FunctionSymbol> funcSymbol;
};

class MemberFuncCallExpr : public FuncCallExpr{
public:
	MemberFuncCallExpr(std::shared_ptr<Expression> _instance, std::shared_ptr<IdentifierExpr> _funcName,
		std::vector<std::shared_ptr<Expression> > _args)
		:FuncCallExpr(_funcName,_args),instance(std::move(_instance)){}

	std::shared_ptr<Expression> getInstance() { return instance; }
	ACCEPT_VISITOR
private:
	std::shared_ptr<Expression> instance;
};

class ClassMemberExpr :public Expression {
public:
	ClassMemberExpr(std::shared_ptr<Expression> _obj, std::shared_ptr<IdentifierExpr> _id)
		:obj(std::move(_obj)), identifier(std::move(_id)) {}

	std::shared_ptr<IdentifierExpr> getIdentifier() { return identifier; }
	std::shared_ptr<Expression> getObj() { return obj; }

	std::shared_ptr<VarSymbol> getSymbol() { return symbol; }
	void setSymbol(std::shared_ptr<VarSymbol> _symbol) { symbol = _symbol; }

	ACCEPT_VISITOR
private:
	std::shared_ptr<Expression> obj;
	std::shared_ptr<IdentifierExpr> identifier;

	std::shared_ptr<VarSymbol> symbol;
};

/*
Usage: type array[][]...[] = new type[dim_1][dim_2]...[dim_n][]..[]
*/
class NewExpr : public Expression {
public:
	NewExpr(std::shared_ptr<Type> _type, std::vector<std::shared_ptr<Expression> > ctor_args)
		:type(std::move(_type)){
		auto fName = std::make_shared<Identifier>(type->getIdentifier() + ":ctor");
		ctorCall = std::make_shared<FuncCallExpr>(
			std::make_shared<IdentifierExpr>(fName),ctor_args);
	}

	NewExpr(std::shared_ptr<Type>_type, std::vector<std::shared_ptr<Expression> > v, int _dim)
		:type(std::move(_type)), dimensions(std::move(v)), dimension(_dim) {}

	std::shared_ptr<FuncCallExpr> getCtorCall() { return ctorCall; }
	
	std::shared_ptr<Type> getType() { return type; }
	std::vector<std::shared_ptr<Expression> > &getDimensions() { return dimensions; }

	std::shared_ptr<FunctionSymbol> getConstructor() { return constructor; }
	void setConstructor(std::shared_ptr<FunctionSymbol> ctor) { constructor = ctor; }

	int getNumberOfDim() { return dimension; }
	ACCEPT_VISITOR
private:
	std::shared_ptr<Type> type;
	std::vector<std::shared_ptr<Expression> > dimensions;
	std::shared_ptr<FuncCallExpr> ctorCall;
	int dimension;
	//semantic
	std::shared_ptr<FunctionSymbol> constructor;
};

class ThisExpr :public Expression,public Identifier{
public:
	ThisExpr() :Identifier("this") {}

	std::shared_ptr<ClassSymbol> getCls() { return cls; }
	void setClass(std::shared_ptr<ClassSymbol> _cls) { cls = _cls; }
	ACCEPT_VISITOR
private:
	std::shared_ptr<ClassSymbol> cls;
};

/******************************************Expressions end here************************************************/

class Statement : public virtual astNode{
public:
	ACCEPT_VISITOR_VIRTUAL
};

/*
Interface class <Declararion>
Since variable declartion might include initial value,
we need to recongnize it from other declartions,
and hence we have the method <isVarDecl>
*/
class Declaration : public virtual astNode {
public:
	virtual bool isVarDecl() { return false; }
	virtual bool isFuncDecl() { return false; }
	ACCEPT_VISITOR_VIRTUAL
};

/*
Note: This is baseClass for ForStmt and WhileStmt.
It is designed for necessary information in IR generation.
*/
class Loop {
public:
	void setStartBlk(const std::shared_ptr<BasicBlock> &st) { start = st; }
	std::shared_ptr<BasicBlock> getStartBlk() { return start; }

	void setFinalBlk(const std::shared_ptr<BasicBlock> &blk) { _final = blk; }
	std::shared_ptr<BasicBlock> getFinalBlk() { return _final; }
private:
	std::shared_ptr<BasicBlock> start, _final;
};

/*****************************************Base classes end here*********************************************/

//Particularly, <VarDeclStmt> belongs to <Statement> and <Declaration> at the same time.
class VarDeclStmt : public Statement, public Declaration{
public:
	VarDeclStmt(std::shared_ptr<Type> _type,
		std::shared_ptr<Identifier> _id,
		std::shared_ptr<Expression> _init = nullptr)
		: type(std::move(_type)), identifier(std::move(_id)), init(std::move(_init)) {}


	std::shared_ptr<Expression> getInitExpr() { return init; }
	std::shared_ptr<Type> getType() { return type; }
	std::shared_ptr<Identifier> getIdentifier() { return identifier; }
	std::shared_ptr<VarSymbol> getVarSymbol() { return varSymbol; }
	std::shared_ptr<SymbolType> getSymbolType() { return typeOfSymbol; }
	void setVarSymbol(std::shared_ptr<VarSymbol> _varSymbol) { varSymbol = _varSymbol; }
	void setSymbolType(std::shared_ptr<SymbolType> _type) { typeOfSymbol = _type; }
	void setInitExpr(std::shared_ptr<Expression> _init) { init = _init; }

	bool isVarDecl() override { return true; }

	//IR
	void markAsArg() { isArg = true; }
	bool isArgument() { return isArg; }
	ACCEPT_VISITOR
private:
	std::shared_ptr<Type> type;
	std::shared_ptr<Identifier> identifier;
	std::shared_ptr<Expression> init;

	std::shared_ptr<VarSymbol> varSymbol;
	std::shared_ptr<SymbolType> typeOfSymbol;

	//IR
	std::shared_ptr<Register> reg;
	bool isArg;
};

class IfStmt : public Statement {
public:
	IfStmt(std::shared_ptr<Expression> _condition,
		std::shared_ptr<Statement> _then,
		std::shared_ptr<Statement> _ELSE = nullptr)
		:condition(_condition), then(_then), ELSE(_ELSE) {}
	
	std::shared_ptr<Expression> getCondition() { return condition; };
	std::shared_ptr<Statement> getThen() { return then;}
	std::shared_ptr<Statement> getElse() { return ELSE;}
	ACCEPT_VISITOR
private:
	std::shared_ptr<Expression> condition;
	std::shared_ptr<Statement> then;
	std::shared_ptr<Statement> ELSE;
};

class ForStmt : public Statement, public Loop{
public:
	ForStmt(std::shared_ptr<Statement> _init, std::shared_ptr<Expression> _cond,
		std::shared_ptr<Statement> _iter, std::shared_ptr<Statement> _body)
		:init(std::move(_init)), condition(std::move(_cond)), iter(std::move(_iter)),
			body(std::move(_body)) {}
	std::shared_ptr<Expression> getCondition() { return condition; }
	std::shared_ptr<Statement> getInit() { return init; }
	std::shared_ptr<Statement> getBody() { return body; }
	std::shared_ptr<Statement> getIter() { return iter; }
	ACCEPT_VISITOR
private:
	std::shared_ptr<Statement> init, iter, body;
	std::shared_ptr<Expression> condition;
};

class WhileStmt : public Statement, public Loop{
public:
	WhileStmt(std::shared_ptr<Expression> _condition,
		std::shared_ptr<Statement> _body) 
		:condition(std::move(_condition)), body(std::move(_body)) {}
	
	std::shared_ptr<Expression> getCondition() { return condition;}
	std::shared_ptr<Statement> getBody() { return body; }
	ACCEPT_VISITOR
private:
	std::shared_ptr<Expression> condition;
	std::shared_ptr<Statement> body;
};

class ExprStmt : public Statement{
public:
	ExprStmt(std::shared_ptr<Expression> _expr) :expr(std::move(_expr)) {};

	std::shared_ptr<Expression> getExpr() { return expr; }
	ACCEPT_VISITOR
private:
	std::shared_ptr<Expression> expr;
};

class ReturnStmt : public Statement{
public:
	ReturnStmt(std::shared_ptr<Expression> _value = nullptr) :value(std::move(_value)) {}
	
	std::shared_ptr<Expression> getValue() { return value; }
	std::shared_ptr<FunctionSymbol> getFuncSymbol() { return funcSymbol; }
	void setFuncSymbol(std::shared_ptr<FunctionSymbol> _funcSymbol) { funcSymbol = _funcSymbol; }
	ACCEPT_VISITOR
private:
	std::shared_ptr<Expression> value; // value == nullptr if return void type.
	std::shared_ptr<FunctionSymbol> funcSymbol;
};

class BreakStmt : public Statement{
public:
	void setLoop(Loop *_loop) { loop = _loop; }
	Loop  *getLoop() { return loop; }
	ACCEPT_VISITOR
private:
	Loop  *loop; // the related loop
};

class ContinueStmt : public Statement{
public:
	void setLoop(Loop *_loop) { loop = _loop; }
	Loop *getLoop() { return loop; }
	ACCEPT_VISITOR
private:
	Loop *loop; // the related loop
};

// EmptyStmt: Semicolon
class EmptyStmt : public Statement{
public:
	ACCEPT_VISITOR
};

class StmtBlock : public Statement{
public:
	StmtBlock() = default;
	StmtBlock(std::vector<std::shared_ptr<Statement> > _stmts)
		:stmts(std::move(_stmts)) {}

	std::vector<std::shared_ptr<Statement> > &getStmts() { return stmts; }
	ACCEPT_VISITOR
private:
	std::vector<std::shared_ptr<Statement> > stmts;
};

/******************************************Statements end here************************************************/
class MultiVarDecl : public Statement, public Declaration {
public:
	MultiVarDecl(std::vector<std::shared_ptr<VarDeclStmt> > _decls) :decls(_decls) {}

	std::vector<std::shared_ptr<VarDeclStmt> > &getDecls() { return decls; }

	bool isVarDecl() override { return true; }
	ACCEPT_VISITOR
private:
	std::vector<std::shared_ptr<VarDeclStmt> > decls;
};

class FunctionDecl : public Declaration{
public:
	FunctionDecl(std::shared_ptr<Type> _retType,
		std::shared_ptr<Identifier> _name,
		std::vector< std::shared_ptr<VarDeclStmt> > _args,
		std::shared_ptr<StmtBlock> _body)
		:retType(std::move(_retType)),name(std::move(_name)), 
		args(std::move(_args)), body(std::move(_body)) {
		if (name->name.length() >= 5 && name->name.substr(name->name.length() - 5, 5) == ":ctor")
			isCtor = true;
		else isCtor = false;
	}
	
	/*getters/setters*/
	std::shared_ptr<Type> getRetType() { return retType; }
	std::shared_ptr<Identifier> getIdentifier() { return name; }
	std::vector< std::shared_ptr<VarDeclStmt> > &getArgs() { return args; }
	std::shared_ptr<StmtBlock> getBody() { return body; }

	std::shared_ptr<FunctionSymbol> getFuncSymbol() { return funcSymbol; }
	void setFuncSymbol(std::shared_ptr<FunctionSymbol> _funcSymbol) { funcSymbol = _funcSymbol; }
	
	bool isConstructor() { return isCtor; }

	bool isFuncDecl() override { return true; }

	ACCEPT_VISITOR
private:
	//retType == nullptr if this function return with void type.
	std::shared_ptr<Type> retType;
	std::shared_ptr<Identifier> name;
	std::vector< std::shared_ptr<VarDeclStmt> > args;
	std::shared_ptr<StmtBlock> body;

	std::shared_ptr<FunctionSymbol> funcSymbol;
	bool isCtor;
};

class ClassDecl : public Declaration{
public:
	ClassDecl(std::shared_ptr<Identifier> _name,
		std::vector< std::shared_ptr<Declaration> > _members)
		:name(std::move(_name)), members(std::move(_members)) {}
	
	std::shared_ptr<Identifier> getIdentifier() { return name; }
	std::vector< std::shared_ptr<Declaration> > &getMembers(){ return members; }

	std::shared_ptr<ClassSymbol> getClsSymbol() { return clsSymbol; }
	void setClsSymbol(std::shared_ptr<ClassSymbol> _clsSymbol) { clsSymbol = _clsSymbol; }

	ACCEPT_VISITOR
private:
	std::shared_ptr<Identifier> name;
	std::vector< std::shared_ptr<Declaration> > members;
	
	std::shared_ptr<ClassSymbol> clsSymbol;
};

/**************************************************Program****************************************************/
class ProgramAST :public astNode{
public:
	ProgramAST(std::vector<std::shared_ptr<Declaration> > _decls) 
		:decls(std::move(_decls)) {}
	ACCEPT_VISITOR

	std::vector<std::shared_ptr<Declaration> > &getDecls(){ return decls; }
private:
	std::vector<std::shared_ptr<Declaration> > decls;
};
