#pragma once

#include "pch.h"
#include "RISCVassembly.h"
#include "configuration.h"
#include <assert.h>
#include <set>

class RegisterAllocator {
public:
	RegisterAllocator(std::shared_ptr<RISCVProgram> _program) 
		:program(_program) {
		for (auto regName : RISCVConfig::physicalRegNames) {
			preColored.insert((*program)[regName]);
		}
		K = RISCVConfig::allocatableRegNames.size();
	}

	~RegisterAllocator();

	using Edge = std::pair<std::shared_ptr<Register>, std::shared_ptr<Register> >;
	
	void run();
private:
	static const int inf = 0x3f3f3f3f;
	int K;

	std::shared_ptr<RISCVProgram> program;

	std::set<std::shared_ptr<Register> > preColored, initial, 
		simplifySet, freezeSet, spillSet,
		spilled, coalesced, colored;
	
	std::set<std::shared_ptr<MoveAssembly> >
		coalescedMoves, constrainedMoves, frozenMoves, activeMoves, moveSet;

	std::map<std::shared_ptr<RISCVBasicBlock>,
		std::set<std::shared_ptr<Register> > > livein, liveout, def, use;

	// spill
	std::map<std::shared_ptr<Register>, bool> isForSpill;
	std::map<std::shared_ptr<Register>, int> spillPriority;

	std::vector<std::shared_ptr<Register> > selectStack;

	std::set<Edge> edges;
	
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
	void apply();

	std::shared_ptr<RISCVFunction> f;

	/***************************** helper functions******************************/
	void clear();

	void addEdge(std::shared_ptr<Register> x, std::shared_ptr<Register> y);

	bool isMoveRelated(std::shared_ptr<Register> reg);

	std::set<std::shared_ptr<MoveAssembly> > nodeMoves(std::shared_ptr<Register> reg);

	std::set<std::shared_ptr<Register> > getNeighbors(std::shared_ptr<Register> reg);

	void decreaseDegreeBy1(std::shared_ptr<Register> reg);

	void enableMove(std::set<std::shared_ptr<Register> > regs);

	std::shared_ptr<Register> getAlias(std::shared_ptr<Register> reg);

	void enqueue(std::shared_ptr<Register> reg);

	bool check(std::shared_ptr<Register> t, std::shared_ptr<Register> r);

	bool isConservative(std::set<std::shared_ptr<Register> > regs);

	void union_nodes(std::shared_ptr<Register> x, std::shared_ptr<Register> y);

	void freezeMoves(std::shared_ptr<Register> r);

};