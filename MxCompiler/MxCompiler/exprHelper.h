#pragma		once
#include "astnode.h"
#include "token.h"

class OperatorCategory {
public:
	OperatorCategory() {
		init();
	}

	const std::unordered_map<Tag, BinaryExpr::Operator> & additive() { return Additive; }
	const std::unordered_map<Tag, BinaryExpr::Operator> & multiplicative() { return Multiplicative; }
	const std::unordered_map<Tag, BinaryExpr::Operator> & rel() { return Rel; }
	const std::unordered_map<Tag, BinaryExpr::Operator> & shift() { return Shift; }
	const std::unordered_map<Tag, BinaryExpr::Operator> & equality() { return Equality; }

private:
	std::unordered_map<Tag, BinaryExpr::Operator> Equality, Additive, Multiplicative, Rel, Shift;

	void init() {
		Additive = { 
			{Multiply,BinaryExpr::TIMES},
			{Divide, BinaryExpr::DIVIDE},
			{Mod, BinaryExpr::MOD} 
		};

		Multiplicative = {
			{Add, BinaryExpr::ADD},
			{Minus, BinaryExpr::MINUS}
		};

		Rel = {
			{Less, BinaryExpr::LESS},
			{Greater, BinaryExpr::GREATER},
			{Leq, BinaryExpr::LEQ},
			{Geq,BinaryExpr::GEQ},
		};

		Shift = {
			{LShift, BinaryExpr::LSHIFT},
			{RShift, BinaryExpr::RSHIFT}
		};

		Equality = {
			{Equal, BinaryExpr::EQ},
			{Neq, BinaryExpr::NEQ}
		};
	}
};