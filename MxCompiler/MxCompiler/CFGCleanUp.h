#pragma once

#include "cfg_pass.h"

/*
This class will clean up redundant basic blocks in the CFG.
*/

class CFGCleanUpPass : public CFG_Pass{
public:
	CFGCleanUpPass(std::shared_ptr<IR> ir) :CFG_Pass(ir) {}

	bool run() override;
private:
};
