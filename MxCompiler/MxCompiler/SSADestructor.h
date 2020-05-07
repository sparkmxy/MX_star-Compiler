#pragma once

#include "cfg_pass.h"
#include "IR.h"
#include "dominance.h"


class SSADestructor: public CFG_Pass{
public:
	SSADestructor(std::shared_ptr<IR> ir) :CFG_Pass(ir) {}

	bool run() override;

private:

	struct ParallelCopy {
		std::shared_ptr<VirtualReg> dst;
		std::shared_ptr<Operand> src;

		ParallelCopy(std::shared_ptr<VirtualReg> _dst, std::shared_ptr<Operand> _src)
			:dst(_dst), src(_src) {}
	};

	std::unordered_map<std::shared_ptr<BasicBlock>, std::unordered_set<ParallelCopy> > parallel_copy;

	void removePhiFunction(std::shared_ptr<Function> f);
	void replaceParalleCopy(std::shared_ptr<Function> f);
};

