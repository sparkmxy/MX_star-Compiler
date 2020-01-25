#pragma once

#include "pch.h"

class Error {
public:
	Error(const std::string &_msg) :msg(_msg) {}
	virtual std::string what() { return msg; }

protected:
	std::string msg;
};

class SyntaxError : public Error {
public:
	SyntaxError(const std::string &msg, const Position &_pos)
		:Error(msg), pos(_pos) {}
	std::string what() { return msg + " at " + pos.toString(); }
private:
	Position pos;
};


