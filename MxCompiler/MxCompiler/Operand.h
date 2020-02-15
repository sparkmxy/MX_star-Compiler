#pragma once

#include "pch.h"

/*
Class Operand: an interface class
*/
class Operand {
public:
	enum Category
	{
		REG_REF, IMM, REG_VAL, STATICSTR
	};
	virtual Category category() = 0;
};

/*
Class : Register
The variables that has real address in the RAM should
be tagged as <REG_REF>.
On the contrary, <REG_VAL> means
it is just a temperory value in the register.
*/
class Register : public Operand {
public:
	Register() {}
	Register(Category _tag, std::string _name) : name(_name),tag(_tag){}
	std::string getName() { return name; }

	Category category() override { return tag; }
private:
	std::string name;
	Category tag;
};

class Immediate : public Operand {
public:
	Immediate(int _val) : value(_val) {}
	int getValue() { return value; }

	Category category() override { return IMM; }
private:
	int value;
};


class VirtualReg: public Register {
public:
	VirtualReg(Category tag = REG_VAL, std::string _name = "") :Register(tag, _name) {}
};

class StaticString : public Operand {
public:
	StaticString(const std::string &_text) :text(_text) {}

	std::string getText() { return text; }
	Category category() override { return STATICSTR; }
private:
	std::string text;
};