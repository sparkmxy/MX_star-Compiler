#pragma once

#include "pch.h"
#include "Operand.h"

class RISCVFunction;

class PhysicalRegister : public Register {
public:
	PhysicalRegister(const std::string &name) :Register(Operand::PHISICAL, name) {}
};

// Constructor: base, offset, topdown = true

class Address : public Operand {
public:
	Address(int o) : offset(o) {}
	int getOffset() { return offset; }
	virtual void accept(CFG_Visitor &vis) {/*do nothing*/}
protected:
	int offset;
};

class BaseOffsetAddr : public Address {
public:
	BaseOffsetAddr(std::shared_ptr<Register> r, int _offset) : Address(_offset), base(r) {}

	Category category() override { return ADDR; }
	std::shared_ptr<Register> getBase() { return base; }
	void setBase(std::shared_ptr<Register> r) { base = r; }
private:
	std::shared_ptr<Register> base;
};

class StackLocation : public Address {
public:
	StackLocation(std::weak_ptr<RISCVFunction> _f, std::shared_ptr<PhysicalRegister> _sp,
		int _offset, bool _topdown = true)
		:Address(_offset), sp(_sp), f(_f), topdown(_topdown) {
		if (!_topdown) 
			offset = -offset;
	}
	
	Category category() override { return STACK; }

	bool isTopdown() { return topdown; }
	
	std::shared_ptr<PhysicalRegister> getSp() { return sp; }
	std::shared_ptr<RISCVFunction> getFunction() { return f.lock(); }
private:
	std::weak_ptr<RISCVFunction> f;
	std::shared_ptr<PhysicalRegister> sp;
	bool topdown;
};