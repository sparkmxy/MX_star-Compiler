#pragma once
#include "pch.h"
#include "astnode.h"
#include "token.h"


class OperatorCategory {
public:
	OperatorCategory() {
		init();
	}

	std::map<Tag, BinaryExpr::Operator> & additive() { return Additive; }
	std::map<Tag, BinaryExpr::Operator> & multiplicative() { return Multiplicative; }
	std::map<Tag, BinaryExpr::Operator> & rel() { return Rel; }
	std::map<Tag, BinaryExpr::Operator> & shift() { return Shift; }
	std::map<Tag, BinaryExpr::Operator> & equality() { return Equality; }

private:
	std::map<Tag, BinaryExpr::Operator> Equality, Additive, Multiplicative, Rel, Shift;

	void init() {
		Multiplicative = {
			{Multiply,BinaryExpr::TIMES},
			{Divide, BinaryExpr::DIVIDE},
			{Mod, BinaryExpr::MOD} 
		};

		Additive = {
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