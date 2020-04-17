
#pragma once
#include "position.h"

class Error {
public:
	Error(const std::string &_msg)
		:msg(_msg){}
	virtual std::string what() { return "Error: " + msg;}
protected:
	std::string msg;
};

class SyntaxError : public Error {
public:
	SyntaxError(const std::string &_msg, const Position &_pos)
		:Error(_msg), pos(_pos){}
	std::string what() { return "Syntax error: " + msg + " at " + pos.toString(); }
private:
	Position pos;
};


class SemanticError : public Error {
public:
	SemanticError(const std::string &_msg, const Position &_pos)
		:Error(_msg),pos(_pos) {}
	std::string what() override { return "Semantic error: " + msg + " at " + pos.toString(); }
private:
	Position pos;
};
