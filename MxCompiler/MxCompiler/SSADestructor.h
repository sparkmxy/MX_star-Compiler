#pragma once

#include "cfg_pass.h"

class SSADestructor :public CFG_Pass{
public:
	SSADestructor();
	~SSADestructor();
};

