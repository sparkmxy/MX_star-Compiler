#pragma once
#include "pch.h"
#include "cfg_pass.h"

class DeadCodeEliminationPass : public CFG_Pass {
public:
	DeadCodeEliminationPass(std::shared_ptr<IR> ir) :CFG_Pass(ir) {}

	void run();

private:

	void eliminate(std::shared_ptr<Function> f);
	void resolveUseDefChain(std::shared_ptr<Function> f);

};
