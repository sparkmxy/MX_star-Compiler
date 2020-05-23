#pragma once

#include "pch.h"
#include "RISCVassembly.h"

class PeeholeMatchingOptimizer {
public:
	PeeholeMatchingOptimizer(std::shared_ptr<RISCVProgram> p) :program(p) {}

	void run();
private:
	std::shared_ptr<RISCVProgram> program;

};