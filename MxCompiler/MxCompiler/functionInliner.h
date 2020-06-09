#pragma once

#include "pch.h"
#include "IR.h"
#include "IRinstruction.h"

class FunctionInliner {
public:
	FunctionInliner(std::shared_ptr<IR> _ir) :ir(_ir) {}
	void run();
private:

	std::shared_ptr<IR> ir;
	std::map<std::shared_ptr<Function>, int> call_cnt, instr_cnt;

	void prepare();
	void non_recursive();
	void recursive();
};