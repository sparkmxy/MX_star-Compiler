#pragma once

#include "pch.h"
#include "cfg_pass.h"

class CommonSubExpressionElimiator :public CFG_Pass {
public:
	CommonSubExpressionElimiator(std::shared_ptr<IR> _ir) :CFG_Pass(_ir){}

	bool run() override;
private:
	bool changed;
	
	void runForBlock(std::shared_ptr<BasicBlock> b);
};