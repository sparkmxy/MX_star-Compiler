#include "functionInliner.h"

void FunctionInliner::run()
{
	prepare();
	non_recursive();
	recursive();
}

void FunctionInliner::prepare()
{
	call_cnt.clear();
	for (auto &f : ir->getFunctions()) {
		int cnt = 0;
		for(auto b : f->getBlockList())
			for (auto i = b->getFront(); i != nullptr; i = i->getNextInstr()) {
				cnt++;
				if (i->getTag() == IRInstruction::CALL) 
					call_cnt[std::static_pointer_cast<Call>(i)->getFunction()]++;
			}

		instr_cnt[f] = cnt;
	}
}

void FunctionInliner::non_recursive()
{

}

void FunctionInliner::recursive()
{

}
