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
		std::shared_ptr<Register> dst;
		std::shared_ptr<Operand> src; // this can be an immediate

		ParallelCopy(std::shared_ptr<Register> _dst, std::shared_ptr<Operand> _src)
			:dst(_dst), src(_src) {}
	};

	std::map<std::shared_ptr<BasicBlock>, 
		std::set<std::shared_ptr<ParallelCopy> > > parallel_copy;

	void removePhiFunction(std::shared_ptr<Function> f);
	void sequentializeParalleCopy(std::shared_ptr<Function> f);
};

