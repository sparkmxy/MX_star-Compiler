#pragma once

#include "pch.h"
#include "IR.h"

class IR_Printer {
public:
	IR_Printer(std::shared_ptr<IR> _ir) :ir(_ir) {}
private:
	std::shared_ptr<IR> ir;
};