#include "parse.h"
#include "token.h"

namespace ph = std::placeholders;

astNode *Parser::getAST() {
	
}

std::shared_ptr<Identifier> Parser::identifier() { 
	if (getTag(cur) == ID) {
		auto p = (*cur)->pos;
		return newNode<Identifier>(p,(*(cur++))->toString());
	}
	return nullptr;
}

std::shared_ptr<BuiltinType> Parser::builtinType() {
	auto id = getTag(cur) == ID;
	if (id == Num || id == String || id == Bool) {
		auto p = (*cur)->pos;
		switch (id) {
		case Num: return newNode<BuiltinType>(p, (*(cur++))->toString(), BuiltinType::INT);
		case String: return newNode<BuiltinType>(p, (*(cur++))->toString(), BuiltinType::STRING);
		case Bool: return newNode<BuiltinType>(p, (*(cur++))->toString(), BuiltinType::BOOL);
		default:
			throw Error("Parser error: undefined builtin-type.");
		}
	}
	return nullptr;
}

std::shared_ptr<BasicType> Parser::basicType() {
	std::shared_ptr<BasicType> ret = builtinType();
	if (ret != nullptr) return ret;
	ret = identifier();
	if (ret != nullptr) return newNode<UserDefinedType>(node2Pos[ret->id()], ret);
	return nullptr;
}

std::shared_ptr<Type> Parser::type() {
	auto st = cur;
	std::shared_ptr<Type> ret = basicType();
	if (ret == nullptr) return nullptr;
	while ((*cur)->tag() == LeftIndex) {
		cur++;
		if ((*cur)->tag() == RightIndex) 
			ret = newNode<Type>((*st)->pos().first, (*cur)->pos().second, ret);
		else{
			cur = st;
			return nullptr;
		}
		cur++;
	}
	return ret;
}

std::shared_ptr<ConstValue> Parser::constValue() {
	auto tag = (*cur)->tag();
	if (tag == Num || tag == True || tag == False || tag == ConstString || tag == Null) {
		auto p = (*cur)->pos; 
		Token *tkptr = *cur;
		cur++;
		switch (tag)
		{
		case Num: return newNode<NumValue>(p,reinterpret_cast<Number*>(tkptr)->value());
		case ConstString: return newNode<StringValue>(p, tkptr->toString());
		case True: return newNode<BoolValue>(p, true);
		case False: return newNode<BoolValue>(p, false);
		case Null: return newNode<NullValue>(p);
		}
	}
	return nullptr;
}

std::shared_ptr<NewExpr> Parser::newExpr() {
	auto st = (*cur)->pos().first;
	if ((*cur)->tag != New) return nullptr;
	cur++;
	
	std::shared_ptr<Type> baseType = basicType();
	if (baseType == nullptr) return nullptr;
	std::vector<std::shared_ptr<Expression>> dimensions;

	//()
	if ((*cur)->tag == LeftBracket) {
		cur++;
		if ((*cur)->tag != RightBracket)
			throw SyntaxError("Parser error : Missing ')' in new expression", (*cur)->pos().first);
		return newNode<NewExpr>(st, (*(cur++))->pos().second, baseType, dimensions);
	}

	bool noDim = false;
	while ((*cur)->tag == LeftIndex) {
		cur++;
		if ((*cur)->tag == RightIndex) {  // no more dimensions
			noDim = true;
			cur++;
			baseType = newNode<ArrayType>(st, (*cur)->pos().second, baseType);
			continue;
		}
		if (noDim)
			throw SyntaxError("no dimension declaration",(*cur)->pos().first);
		auto expr = expression();
		if (expr == nullptr)
			throw SyntaxError("Parser error: incomplete expression", (*cur)->pos().first);
		dimensions.push_back(expr);
	}
	return newNode<NewExpr>(st, (*cur)->pos().second, baseType, dimensions);
}


std::shared_ptr<Expression> Parser::multiplicativeExpr() {
	return
		expressionHelper(std::bind(&Parser::prefixUnaryExpr, this, ph::_1, ph::_2),
			op.multiplicative);
}

std::shared_ptr<Expression> Parser::additiveExpr() {
	return
		expressionHelper(std::bind(&Parser::multiplicativeExpr, this, ph::_1, ph::_2),
			op.additive);
}

std::shared_ptr<Expression> Parser::shiftExpr() {
	return
		expressionHelper(std::bind(&Parser::additiveExpr, this, ph::_1, ph::_2),
			op.shift);
}


std::shared_ptr<Expression> Parser::relationExpr() {
	return
		expressionHelper(std::bind(&Parser::shiftExpr, this, ph::_1, ph::_2),
			op.rel);
}

std::shared_ptr<Expression> Parser::equalityExpr() {
	return
		expressionHelper(std::bind(&Parser::relationExpr, this, ph::_1, ph::_2),
			op.equality);
}

std::shared_ptr<Expression> Parser::bitAndExpr() {
	return
		expressionHelper(std::bind(&Parser::equalityExpr, this, ph::_1, ph::_2),
			std::make_pair(BitAnd, BinaryExpr::Operator::BITAND));
}

std::shared_ptr<Expression> Parser::bitXorExpr() {
	return
		expressionHelper(std::bind(&Parser::bitAndExpr, this, ph::_1, ph::_2),
			std::make_pair(Xor, BinaryExpr::Operator::BITXOR));
}


std::shared_ptr<Expression> Parser::bitOrExpr() {
	return
		expressionHelper(std::bind(&Parser::bitXorExpr, this, ph::_1, ph::_2),
			std::make_pair(BitOr, BinaryExpr::Operator::BITOR));
}
      
std::shared_ptr<Expression> Parser::logicalOrExpr() {
	return
		expressionHelper(std::bind(&Parser::logicalAndExpr, this, ph::_1, ph::_2),
			std::make_pair(And, BinaryExpr::Operator::AND));
}

std::shared_ptr<Expression> Parser::expression() {
	return
		expressionHelper(std::bind(&Parser::logicalOrExpr, this, ph::_1, ph::_2),
			std::make_pair(Assign, BinaryExpr::Operator::ASSIGN));
}




