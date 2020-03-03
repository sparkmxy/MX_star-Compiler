#pragma once

#include "pch.h"
#include "Function.h"
#include "basicblock.h"

/*
This class use the Lengauer-Tarjan Algorithm to 
figure out the immediate dominator of each basic block(i.e. node in the control flow graph).
It work out the <DTInfo> for each basic block, and provide
a method <isDominating> to judge whether tow blocks have dominance relation.
*/
class DominanceTree {
public:
	DominanceTree(std::shared_ptr<Function> _f) :f(_f) {
		dfs_clock = 0;
		DFS(f->getEntry());
		workOutIdoms();
		dfs_clock = 0;
		buildDominaceTree(f->getEntry());
	}

	bool isDominating(std::shared_ptr<BasicBlock> x, std::shared_ptr<BasicBlock> y);

private:
	std::shared_ptr<Function> f;

	void DFS(std::shared_ptr<BasicBlock> x, std::shared_ptr<BasicBlock> father = nullptr, int dep = 0);
	std::unordered_set<std::shared_ptr<BasicBlock> > visited;
	std::unordered_map<std::shared_ptr<BasicBlock>, std::vector<std::shared_ptr<BasicBlock> > > sdomEqcls;
	std::vector<std::shared_ptr<BasicBlock> > idfn, fa;
	int dfs_clock;
	void workOutIdoms();
	void buildDominaceTree(std::shared_ptr<BasicBlock> x);
	// Union-find
	std::vector<int> S, val;
	void union_find_init(int n);
	int find(int x);
};