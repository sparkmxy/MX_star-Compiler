#pragma once

#include "cfg_pass.h"
#include "basicblock.h"
#include "IRinstruction.h"
#include <queue>
/*
SSA constructor is part of <Optimizer>.
The SSA construction uses a advanced algorithm 
which does not require the precomutation of DF.
*/
class SSAConstructor :public CFG_Pass{
public:
	SSAConstructor(std::shared_ptr<IR> _ir) :CFG_Pass(_ir) {}

	void constructSSA();
private:
	void insertPhiFunction();
	void renameVariables();
	void renameVariables(std::shared_ptr<Function> func);
	void collectVariales();

	std::unordered_set<std::shared_ptr<BasicBlock> >
		computeIteratedDF(const std::vector<std::shared_ptr<BasicBlock> > &S);

	void visit(std::shared_ptr<BasicBlock> y, const std::shared_ptr<BasicBlock> &current_x,
		std::unordered_set<std::shared_ptr<BasicBlock> > &DFplus);
	void insertNode(std::shared_ptr<BasicBlock> x);

	std::unordered_set<std::shared_ptr<BasicBlock>> visited, onceInQueue;
	std::priority_queue<std::shared_ptr<BasicBlock>,
		std::vector<std::shared_ptr<BasicBlock> >, CompareByDepth> workQueue;
};

class CompareByDepth {
	bool reverse;
public:
	bool operator() (const std::shared_ptr<BasicBlock>& lhs, const  std::shared_ptr<BasicBlock>&rhs) const
	{
		return lhs->getDTInfo().depth < rhs->getDTInfo().depth;
	}
};