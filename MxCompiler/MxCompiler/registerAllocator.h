#pragma once

#include "pch.h"
#include "RISCVassembly.h"

class RegisterAllocator {
public:
	RegisterAllocator(std::shared_ptr<RISCVProgram> _program) 
		:program(_program) {}
	
	void run();
private:

	std::shared_ptr<RISCVProgram> program;

	std::unordered_set<std::shared_ptr<Register> > preColored, initial, 
		simplifySet, freezeSet, spillSet,
		spilled, coalesced, colored;
	
	std::unordered_set<std::shared_ptr<MoveAssembly> >
		coalescedMoves, constrainedMoves, frozenMoves, activeMoves, moveSet;
	
	void init();
	void buildInferenceGraph();
	void livenessAnalysis();
	void collect();
	void simplify();
	void coalesce();
	void freeze();
	void select();
	void assignColor();
	void rewrite();

	std::shared_ptr<RISCVFunction> f;
	// helper functions

	void clear();

};