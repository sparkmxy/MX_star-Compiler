#pragma once
#include "pch.h"
#include "token.h"
#include "astnode.h"
#include "exprHelper.h"

class Parser {
public:
	Parser(std::vector<Token*> &_tokens) :tks(_tokens), cur(_tokens.begin()) {}
	astNode *getAST();

private:
 
	OperatorCategory op;
	std::vector<Token*> &tks;
	std::vector<Token*>::iterator cur;
	std::unordered_map<NodeId, PosPair> node2Pos;

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
	/*Helper functions*/
	Tag getTag(std::vector<Token*>::iterator &iter) {
		if (iter == tks.end()) throw Error("Parser error : invalid token.");
		return (*iter)->tag();
	}

	template<class T, class...Args>
	std::shared_ptr<T> newNode(const Position &st, const Position &ed, Args ... args) {
		auto ret = std::shared_ptr<T>(new T(args...));
		node2Pos[ret->id()] = std::make_pair(st, ed);
		return ret;
	}

	template<class T, class...Args>
	std::shared_ptr<T> newNode(const PosPair &pos, Args ... args) {
		auto ret = std::shared_ptr<T>(new T(args...));
		node2Pos[ret->id()] = pos;
		return ret;
	}

	template <class TermParser>
	std::shared_ptr<Expression> expressionHelper(TermParser term,
		const std::unordered_map<Tag, BinaryExpr::Operator> &ops) {
		auto st = (*cur)->pos().first;
		auto ret = term();
		if (ret == nullptr) return nullptr;

		while (ops.find((*cur)->tag()) != ops.end()) {
			BinaryExpr::Operator op = ops[(*cur)->tag()];
			cur++;
			auto oprand2 = term();
			if (oprand2 == nullptr)
				throw SyntaxError("Parser error: missing oprand.");
			ret = newNode<BinaryExpr>(st, node2Pos[oprand2->id()].second, op, ret, oprand2);
		}
		return ret;
	}

	template <class TermParser>
	std::shared_ptr<Expression> expressionHelper(TermParser term,
		const std::pair<Tag, BinaryExpr::Operator> &p) {
		auto st = (*cur)->pos().first;
		auto ret = term();
		if (ret == nullptr) return nullptr;

		while ((*cur)->tag == p.first) {;
			cur++;
			auto oprand2 = term();
			if (oprand2 == nullptr)
				throw SyntaxError("Parser error: missing oprand.");
			ret = newNode<BinaryExpr>(st, node2Pos[oprand2->id()].second, p.second, ret, oprand2);
		}
		return ret;
	}
};