#pragma once

#include "pch.h"


class BasicBlock;

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
	static bool isRegister(Category tag) { return tag == REG_REF || tag == REG_VAL; }
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

class RegForSSA;

class VirtualReg: public Register {
public:
	VirtualReg(Category tag = REG_VAL, std::string _name = "") 
		:Register(tag, _name),reachingDef(nullptr){}

	std::vector<std::shared_ptr<BasicBlock> > &getDefBlocks() { return defBlocks; }
	void append_def_block(std::shared_ptr<BasicBlock> block) { defBlocks.emplace_back(block); }
	std::shared_ptr<RegForSSA> newName(std::shared_ptr<BasicBlock> block) {
		ssaNames.emplace_back(std::make_shared<RegForSSA>(ssaNames.size(),this, block));
		return ssaNames.back();
	}

	std::shared_ptr<RegForSSA> getReachingDef() { return reachingDef; }
	void setReachingDef(const std::shared_ptr<RegForSSA> rdef) { reachingDef = rdef; }
private:
	//for SSA
	std::vector<std::shared_ptr<BasicBlock> > defBlocks;
	std::vector<std::shared_ptr<RegForSSA> > ssaNames;
protected:
	std::shared_ptr<RegForSSA> reachingDef;
};

class RegForSSA :public VirtualReg{
public:
	RegForSSA(int no, VirtualReg *_prototype, std::shared_ptr<BasicBlock> _def)
		:ssaNo(no), prototype(_prototype), def(std::move(_def)){}

	std::shared_ptr<BasicBlock> getDefiningBlock() { return def; }
private:
	int ssaNo;
	VirtualReg *prototype;
	std::shared_ptr<BasicBlock> def;
	
};

class StaticString : public Operand {
public:
	StaticString(std::shared_ptr<VirtualReg> _reg, const std::string &_text) 
		:reg(_reg), text(_text) {}

	std::string getText() { return text; }
	Category category() override { return STATICSTR; }

	std::shared_ptr<VirtualReg> getReg() { return reg; }
	void setReg(std::shared_ptr<VirtualReg> _reg) { reg = _reg; }

private:
	std::string text;
	std::shared_ptr<VirtualReg> reg;
};