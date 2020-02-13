#pragma once

#include "pch.h"

/*
Class Operand: an interface class
*/
class Operand {
	enum Category
	{
		REG_PTR, IMM, REG_VAL
	};
	virtual Category category() = 0;
};

class Register : public Operand {
public:
	Register() {}
	Register(const std::string &_name) : name(_name) {}
	std::string getName() { return name; }
private:
	std::string name;
};

class Immediate : public Operand {
public:
	Immediate(int _val) : value(_val) {}
	int getValue() { return value; }
private:
	int value;
};


class Int64Reg : public Register {
public:
	Int64Reg(const std::string &_name) :Register(_name) {}
};

class Int64Ptr : public Register {
	Int64Ptr(const std::string &_name) :Register(_name) {}
};

class Int64Global : public Register {  // is this necessary?
	Int64Global(const std::string &_name) :Register(_name) {}
};
