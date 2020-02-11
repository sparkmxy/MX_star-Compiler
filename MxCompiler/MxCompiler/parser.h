
#pragma once

#include "pch.h"
#include "token.h"
#include "astnode.h"
#include "exprHelper.h"

/*	
Outline :	This class recieves tokens an build an AST according to the tokens.
Usage	:	parser.getAST()
note	:	getAST() will throw <SyntaxError> if there is any,
			and finished() will return a boolean value,
			indicating whether tokens given are all parsed.
*/
class Parser {
public:
	Parser(std::vector<std::shared_ptr<Token> > &_tokens) :tks(_tokens), cur(_tokens.begin()) {}
	std::shared_ptr<ProgramAST> getAST();
	bool finished() {
		auto str = (*cur)->toString();
		return (*cur)->tag() == FINISH; 
	}

private:
 
	OperatorCategory op;
	std::vector<std::shared_ptr<Token> > &tks;
	std::vector<std::shared_ptr<Token> >::iterator cur;

	/********************************expression***********************************/
	std::shared_ptr<Identifier> identifier();

	// BuiltinType = Int | String | Bool
	std::shared_ptr<BuiltinType> builtinType();

	// BasicType = BuiltinType | Identifier
	std::shared_ptr<BasicType> basicType();

	// Type = BasicType (LeftBracket RightBracket)*
	std::shared_ptr<Type> type();

	// ConstValue = ConstString | True | False | Num | Null
	std::shared_ptr<ConstValue> constValue();

	// NewExpr = New BasicType (LeftBracket RightBracket 
	//							| LeftIndex expr RightIndex (LeftIndex RightIndex)*)
	std::shared_ptr<NewExpr> newExpr();

	std::shared_ptr<ThisExpr> thisExpr();

	std::shared_ptr<Expression> identifierExpr();

	std::shared_ptr<Expression> expression();

	std::shared_ptr<Expression> relationExpr();

	std::shared_ptr<Expression> shiftExpr();

	std::shared_ptr<Expression> additiveExpr();

	std::shared_ptr<Expression> multiplicativeExpr();

	std::shared_ptr<Expression> logicalOrExpr();

	std::shared_ptr<Expression> logicalAndExpr();

	std::shared_ptr<Expression> bitXorExpr();

	std::shared_ptr<Expression> bitOrExpr();

	std::shared_ptr<Expression> bitAndExpr();

	std::shared_ptr<Expression> equalityExpr();

	std::shared_ptr<Expression> prefixUnaryExpr();

	std::shared_ptr<Expression> suffixUnaryExpr();

	/*
	TopPriorityExpr
		= BasicExpr(
		| LeftIdex expr RightIndex
		| Dot IdentifierExpr   
		| Dot Identifier LeftBracket (expr % ',') RightBracket
		)*
	*/
	std::shared_ptr<Expression> topPriorityExpr();

	/*
	BasicExpr = IdentifierExpr
		| Identifier LeftBracket (expr % ',') RightBracket
		| ConstValue
		| newExpr
		| LeftBracket expr RightBracket
	*/
	std::shared_ptr<Expression> basicExpr();
	
	/****************************************Statements*****************************************/

	// VarDeclStmt = Type Identifier (Assign Expression) ? Semicolon
	std::shared_ptr<VarDeclStmt> varDeclStmt(std::shared_ptr<Type> _type);

	// Ifstmt = If LeftBracket expr RightBracket stmt
	std::shared_ptr<IfStmt> ifStmt();

	std::shared_ptr<ReturnStmt> returnStmt();

	std::shared_ptr<BreakStmt> breakStmt();

	std::shared_ptr<ContinueStmt> continueStmt();

	std::shared_ptr<ForStmt> forStmt();

	std::shared_ptr<WhileStmt> whileStmt();

	std::shared_ptr<ExprStmt> exprStmt();

	std::shared_ptr<EmptyStmt> emptyStmt();

	std::shared_ptr<StmtBlock> stmtBlock();

	std::shared_ptr<Statement> statement();
	/******************************************Declarations***********************************************/

	/*
	FunctionDecl = Type Identifier LeftBracket formalDecl RightBracket Statement
	*/
	std::shared_ptr<MultiVarDecl> multiVarDecl();

	std::shared_ptr<FunctionDecl> functionDecl();

	std::shared_ptr<ClassDecl> classDecl();

	std::shared_ptr<Declaration> declaration();

	/**************************************Helper functions*************************************/
	std::vector<std::shared_ptr<Expression> > arguments();

	std::vector<std::shared_ptr<VarDeclStmt> > formalArguments();

	// FormalArgment = Type Identifier
	std::shared_ptr<VarDeclStmt> formalArgument();

	std::shared_ptr<Statement> singleStmtOrBlock();

	std::shared_ptr<FunctionDecl> constructor(const std::string &className);

	template<class T, class...Args>
	std::shared_ptr<T> newNode(const Position &st, const Position &ed, Args ... args) {
		auto ret = std::shared_ptr<T>(new T(args...));
		ret->setPos(std::make_pair(st, ed));
//		std::clog << "newNode between " << st.toString() << " and " << ed.toString() << '\n';
		return ret;
	}

	template<class T, class...Args>
	std::shared_ptr<T> newNode(const PosPair &pos, Args ... args) {
		auto ret = std::shared_ptr<T>(new T(args...));
		ret->setPos(pos);
//		std::clog << "newNode between " << pos.first.toString() << " and " << pos.second.toString() << '\n';
		return ret;
	}

	template <class TermParser>
	std::shared_ptr<Expression> expressionHelper(TermParser term,
		const std::unordered_map<Tag, BinaryExpr::Operator> &ops) {
		auto st = (*cur)->pos().first;
		auto ret = term();
		if (ret == nullptr) return nullptr;

		while (ops.find((*cur)->tag()) != ops.end()) {
			BinaryExpr::Operator op = ops.find((*cur)->tag())->second;
			cur++;
			auto oprand2 = term();
			if (oprand2 == nullptr)
				throw SyntaxError("Parser error: missing oprand.", (*cur)->pos().first);
			ret = newNode<BinaryExpr>(st, oprand2->endPos(), op, ret, oprand2);
		}
		return ret;
	}

	template <class TermParser>
	std::shared_ptr<Expression> expressionHelper(TermParser term,
		const std::pair<Tag, BinaryExpr::Operator> &p) {
		auto st = (*cur)->pos().first;
		auto ret = term();
		if (ret == nullptr) return nullptr;

		while ((*cur)->tag() == p.first) {
			cur++;
			auto oprand2 = term();
			if (oprand2 == nullptr)
				throw SyntaxError("Parser error: missing oprand.", (*cur)->pos().first);
			ret = newNode<BinaryExpr>(st, oprand2->endPos(), p.second, ret, oprand2);
		}
		return ret;
	}
};