#pragma once

#include "cfg_pass.h"

/*
SSA constructor is part of <Optimizer>.
The SSA construction uses a advanced algorithm 
which does not require the precomutation of DF.
*/
class SSAConstructor :public CFG_Pass{
public:
	SSAConstructor(std::shared_ptr<IR> _ir) :CFG_Pass(_ir) {}

	void constructSSA();
private:
	void insertPhiFunction();
	void renameVariables();
	void collectVariales();
};