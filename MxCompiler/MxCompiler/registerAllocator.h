#pragma once

#include "pch.h"
#include "RISCVassembly.h"
#include "configuration.h"

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

	struct Edge
	{
		std::shared_ptr<Register> x, y;

		Edge() {}
		Edge(std::shared_ptr<Register> _x, std::shared_ptr<Register> _y) : x(_x), y(_y) {}
	};
	
	void run();
private:
	static const int inf = 0x3f3f3f3f;
	int K;

	std::shared_ptr<RISCVProgram> program;

	std::unordered_set<std::shared_ptr<Register> > preColored, initial, 
		simplifySet, freezeSet, spillSet,
		spilled, coalesced, colored;
	
	std::unordered_set<std::shared_ptr<MoveAssembly> >
		coalescedMoves, constrainedMoves, frozenMoves, activeMoves, moveSet;

	std::unordered_map<std::shared_ptr<RISCVBasicBlock>,
		std::unordered_set<std::shared_ptr<Register> > > livein, liveout, def, use;

	// spill
	std::unordered_map<std::weak_ptr<Register>, bool> isForSpill;
	std::unordered_map<std::weak_ptr<Register>, int> spillPriority;

	std::vector<std::shared_ptr<Register> > selectStack;

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

	/***************************** helper functions******************************/
	void clear();

	void addEdge(std::shared_ptr<Register> x, std::shared_ptr<Register> y);

	bool isMoveRelated(std::shared_ptr<Register> reg);

	std::unordered_set<std::shared_ptr<MoveAssembly> > nodeMoves(std::shared_ptr<Register> reg);

	std::unordered_set<std::shared_ptr<Register> > getNeighbors(std::shared_ptr<Register> reg);

	void decreaseDegreeBy1(std::shared_ptr<Register> reg);

	void enableMove(std::unordered_set<std::shared_ptr<Register> > regs);

	std::shared_ptr<Register> getAlias(std::shared_ptr<Register> reg);

	void enqueue(std::shared_ptr<Register> reg);

	bool check(std::shared_ptr<Register> t, std::shared_ptr<Register> r);

	bool isConservative(std::unordered_set<std::shared_ptr<Register> > regs);

	void union_nodes(std::shared_ptr<Register> x, std::shared_ptr<Register> y);

	void freezeMoves(std::shared_ptr<Register> r);

};