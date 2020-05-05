#pragma once

#include "cfg_pass.h"
#include "IR.h"

class SSADestructor: public CFG_Pass{
public:
	SSADestructor(std::shared_ptr<IR> ir) :CFG_Pass(ir) {}

	bool run() override;

private:
};

