#pragma once

#include "pch.h"
#include "Operand.h"

class PhysicalRegister : public Register {
public:
	PhysicalRegister(const std::string &name) :Register(Operand::PHISICAL, name) {}
};
