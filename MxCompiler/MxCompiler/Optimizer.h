#pragma once

#include "SSAConstructor.h"
#include "SSADestructor.h"
#include "dead_code_elimination.h"
#include "ConstantExpressionEvaluation.h"

/*
Class : Optimizer
Note  :	This Class encapsulte the low-level optimizations,
		and it serves as a component of the <MxCompiler> class in the top-level design.
		The Optimizations are:
*/
class Optimizer{
public:
	Optimizer(std::shared_ptr<IR> _ir) :ir(_ir) {}

	void optimize();
private:
	std::shared_ptr<IR> ir;
};

