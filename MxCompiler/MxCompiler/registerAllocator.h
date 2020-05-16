#pragma once

#include "pch.h"
#include "IR.h"

class RegisterAllocator {
public:
	RegisterAllocator(std::shared_ptr<IR> _ir) :ir(_ir) {}
	
	void run();
private:

	std::shared_ptr<IR> ir;
};