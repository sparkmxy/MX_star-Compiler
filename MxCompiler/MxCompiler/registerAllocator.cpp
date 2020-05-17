#include "registerAllocator.h"

void RegisterAllocator::run()
{
	auto functions = program->getFunctions();
	for (auto &function : functions) {
		f = function;
		while (true)
		{
			init();
			livenessAnalysis();
			buildInferenceGraph();
			collect();
			while (!simplifySet.empty() || !moveSet.empty() || freezeSet.empty() || spillSet.empty()) {
				if (!simplifySet.empty()) simplify();
				else if (!moveSet.empty()) coalesce();
				else if (!freezeSet.empty()) freeze();
				else select();
			}
			assignColor();
			if (spilled.empty()) break;
			rewrite();
		}
	}
}

void RegisterAllocator::init()
{
	clear();
}

void RegisterAllocator::clear()
{
	initial.clear();
	coalesced.clear();
	colored.clear();
	
	coalescedMoves.clear();
	constrainedMoves.clear();
	frozenMoves.clear();
	activeMoves.clear();
}
