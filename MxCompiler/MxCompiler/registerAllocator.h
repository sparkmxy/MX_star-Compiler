#pragma once

#include "pch.h"
#include "RISCVassembly.h"

class RegisterAllocator {
public:
	RegisterAllocator(std::shared_ptr<RISCVProgram> _program) 
		:program(_program) {
		for (auto regName : RISCVConfig::physicalRegNames) {
			preColored.insert((*program)[regName]);
		}
	}

	struct Edge
	{
		std::shared_ptr<Register> x, y;

		Edge(std::shared_ptr<Register> _x, std::shared_ptr<Register> _y) : x(_x), y(_y) {}
	};
	
	void run();
private:
	static const int inf = 0x3f3f3f3f;

	std::shared_ptr<RISCVProgram> program;

	std::unordered_set<std::shared_ptr<Register> > preColored, initial, 
		simplifySet, freezeSet, spillSet,
		spilled, coalesced, colored;
	
	std::unordered_set<std::shared_ptr<MoveAssembly> >
		coalescedMoves, constrainedMoves, frozenMoves, activeMoves, moveSet;

	std::unordered_map<std::weak_ptr<RISCVBasicBlock>,
		std::unordered_set<std::shared_ptr<Register> > > livein, liveout, def, use;

	std::unordered_set<Edge> edges;
	
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

	void addEdge(std::shared_ptr<Register> x, std::shared_ptr<Register> y);

};