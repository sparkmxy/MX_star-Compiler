
#pragma once
#include "position.h"

class Error {
public:
	Error(const std::string &_msg, const Position &_pos)
		:msg(_msg), pos(_pos) {}
	virtual std::string what() { return "Error: " + msg;}
protected:
	std::string msg;
	Position pos;
};

class SyntaxError : public Error {
public:
	SyntaxError(const std::string &_msg, const Position &_pos)
		:Error(_msg,_pos){}
	std::string what() { return "Syntax error: " + msg + " at " + pos.toString(); }
};


class SemanticError : public Error {
public:
	SemanticError(const std::string &_msg, const Position &_pos)
		:Error(_msg, _pos) {}
	std::string what() override { return "Semantic error: " + msg + " at " + pos.toString(); }
};
