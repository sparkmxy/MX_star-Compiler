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

	bool changed;

	void omitTrivialBlocks(std::shared_ptr<Function> f);
	void rewriteTrivialBranches(std::shared_ptr<Function> f);
	void convertMultiplyByConst(std::shared_ptr<Function> f);
	void mergeBlockChain(std::shared_ptr<Function> f);

	int _log2(int x) {
		int l = 0;
		while ((1 << (l + 1)) <= x)l++;
		return l;
	}
};
