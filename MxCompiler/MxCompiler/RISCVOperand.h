#pragma once

#include "pch.h"
#include "Operand.h"

class PhysicalRegister : public Register {
public:
	PhysicalRegister(const std::string &name) :Register(Operand::PHISICAL, name) {}
};

// Constructor: base, offset, topdown = true
class StackLocation : public Operand {
public:
	StackLocation(std::shared_ptr<PhysicalRegister> _base, int _offset, bool _topdown = true)
		:base(_base), offset(_offset), topdown(_topdown) {}
	Category category() override { return STACK; }

	bool isTopdown() { return topdown; }
	std::shared_ptr<PhysicalRegister> getBase() { return base; }

	int getOffset() { return offset; }
private:
	std::shared_ptr<PhysicalRegister> base;
	int offset;
	bool topdown;
};