#pragma once

#include "pch.h"
#include "cfg_pass.h"
#include "basicblock.h"
#include "IRinstruction.h"
#include "Operand.h"
#include "dominance.h"
#include <queue>

/*
SSA constructor is part of <Optimizer>.
The SSA construction uses a advanced algorithm 
which does not require the precomutation of DF.
This algorithm is elaborated in Chapter 4 of Static Single Assignment Book.
*/
class CompareByDepth {
public:
	bool operator() (const std::shared_ptr<BasicBlock>& lhs, const  std::shared_ptr<BasicBlock>&rhs) const
	{
		return lhs->getDTInfo().depth < rhs->getDTInfo().depth;
	}
};

class SSAConstructor :public CFG_Pass{
public:
	SSAConstructor(std::shared_ptr<IR> _ir) :CFG_Pass(_ir){}

	bool run() override;
private:
	void insertPhiFunction();
	void renameVariables();
	void renameVariables(std::shared_ptr<Function> func);
	void collectVariales();

	void updateReachingDef
	(std::shared_ptr<VirtualReg> v, std::shared_ptr<IRInstruction> i, std::shared_ptr<Function> f);

	std::set<std::shared_ptr<BasicBlock> >
		computeIteratedDF(const std::vector<std::shared_ptr<BasicBlock> > &S);

	void visit(std::shared_ptr<BasicBlock> y, const std::shared_ptr<BasicBlock> &current_x,
		std::set<std::shared_ptr<BasicBlock> > &DFplus);
	void insertNode(std::shared_ptr<BasicBlock> x);

	std::set<std::shared_ptr<BasicBlock>> visited, onceInQueue;
	std::priority_queue<std::shared_ptr<BasicBlock>,
		std::vector<std::shared_ptr<BasicBlock> >, CompareByDepth> workQueue;
};

