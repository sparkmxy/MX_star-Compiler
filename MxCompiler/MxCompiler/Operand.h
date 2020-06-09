#pragma once

#include "pch.h"
#include "cfg_visitor.h"
#include <set>

class BasicBlock;
class Register;
class MoveAssembly;
class StackLocation;
class PhysicalRegister;
/*
This struct is for register allocation information
*/
struct RAinfo {
	// This might causes memory leak, remember to destruct these set
	std::set<std::shared_ptr<Register> > adjList;
	std::set<std::shared_ptr<MoveAssembly> > moveList;
	int degree;
	std::shared_ptr<Register> alias;
	std::shared_ptr<PhysicalRegister> color;
	std::shared_ptr<StackLocation> spilladdr;

	RAinfo() { clear(); }
	void clear() {
		color = nullptr;
		degree = 0;

		adjList.clear();
		moveList.clear();
		alias = nullptr;
		spilladdr = nullptr;
	}
};

/*
Class Operand: an interface class
*/
class Operand {
public:
	enum Category
	{
		REG_REF, IMM, REG_VAL, STATICSTR,
		PHISICAL, STACK, ADDR
	};
	virtual Category category() = 0;
	static bool isRegister(Category tag) { return tag == REG_REF || tag == REG_VAL || tag == PHISICAL; }
	virtual void accept(CFG_Visitor &vis) = 0;
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
	Register(): global(false){}
	Register(Category _tag, std::string _name) : name(_name),tag(_tag),global(false), isForStaticStr(false){
		if (!isRegister(_tag)) throw Error("creating illeagal register.");
	}
	std::string getName() { return name; }

	Category category() override { return tag; }

	bool isGlobal() { return global; }
	void markAsGlobal() { global = true; }

	void markAsStaticString() { global = isForStaticStr = true; }
	bool isForStaticString() { return isForStaticStr; }

	RAinfo &info() { return raInfo; }

	ACCEPT_CFG_VISITOR
private:
	bool global, isForStaticStr;
	std::string name;
	Category tag;

	RAinfo raInfo;
	// For codegen

};

class Immediate : public Operand {
public:
	Immediate(int _val) : value(_val) {}
	int getValue() { return value; }
	Category category() override { return IMM; }

	ACCEPT_CFG_VISITOR
private:
	int value;
};

class RegForSSA;

class VirtualReg: public Register {
public:
	VirtualReg(Category tag = REG_VAL, std::string _name = "") 
		:Register(tag, _name){}

	std::vector<std::shared_ptr<BasicBlock> > &getDefBlocks() { return defBlocks; }
	void append_def_block(std::shared_ptr<BasicBlock> block) { defBlocks.emplace_back(block); }
	std::shared_ptr<RegForSSA> newName(std::shared_ptr<BasicBlock> block) {
		ssaNames.emplace_back(std::make_shared<RegForSSA>(ssaNames.size(),this, block));
		return ssaNames.back();
	}

	std::shared_ptr<RegForSSA> getReachingDef() {  
		return reachingDef;
	}
	void setReachingDef(std::shared_ptr<RegForSSA> rdef) {reachingDef = rdef;}
private:
	//for SSA
	std::vector<std::shared_ptr<BasicBlock> > defBlocks;
	std::vector<std::shared_ptr<RegForSSA> > ssaNames;
protected:
	std::shared_ptr<RegForSSA> reachingDef;
};

class RegForSSA :public VirtualReg{
public:
	RegForSSA(int no, VirtualReg *_prototype, std::weak_ptr<BasicBlock> _def)
		:ssaNo(no), prototype(_prototype), def(std::move(_def)){}

	std::shared_ptr<BasicBlock> getDefiningBlock() { return def.lock(); }
private:
	int ssaNo;
	VirtualReg *prototype;
	std::weak_ptr<BasicBlock> def;
	
};

class StaticString : public Operand {
public:
	StaticString(std::shared_ptr<VirtualReg> _reg, const std::string &_text) 
		:reg(_reg), text(_text) {}

	std::string getText() { return text; }
	Category category() override { return STATICSTR; }

	std::shared_ptr<VirtualReg> getReg() { return reg; }
	void setReg(std::shared_ptr<VirtualReg> _reg) { reg = _reg; }

	ACCEPT_CFG_VISITOR
private:
	std::string text;
	std::shared_ptr<VirtualReg> reg;
};