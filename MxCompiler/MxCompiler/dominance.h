#pragma once

#include "pch.h"
#include "Function.h"
#include "basicblock.h"

/*
This class use an efficient algorithm, the Lengauer-Tarjan Algorithm,  to 
figure out the immediate dominator of each basic block(i.e. node in the control flow graph).
It work out the <DTInfo> for each basic block, and provide
a method <isDominating> to judge whether tow blocks have dominance relation.
*/
class DominatorTree {
public:
	DominatorTree(std::shared_ptr<Function> _f) :f(_f) {
		dfs_clock = 0;
		DFS(f->getEntry());
		union_find_init(dfs_clock + 1);
		workOutIdoms();
		dfs_clock = 0;
		f->getBlockList().clear();
		buildDJGraph(f->getEntry());
	}

	bool isDominating(std::shared_ptr<BasicBlock> x, std::shared_ptr<BasicBlock> y);
	bool isStrictlyDominating(std::shared_ptr<BasicBlock> x, std::shared_ptr<BasicBlock> y);
private:
	std::shared_ptr<Function> f;

	void DFS(std::shared_ptr<BasicBlock> x, std::shared_ptr<BasicBlock> father = nullptr, int dep = 0);
	std::set<std::shared_ptr<BasicBlock> > visited;
	std::map<std::shared_ptr<BasicBlock>, std::vector<std::shared_ptr<BasicBlock> > > sdomEqcls;
	std::vector<std::shared_ptr<BasicBlock> > idfn, fa;
	int dfs_clock;
	void workOutIdoms();
	void buildDJGraph(std::shared_ptr<BasicBlock> x);

	//return true if x strictly dominates y

	// Union-find
	std::vector<int> S, val;
	void union_find_init(int n);
	int find(int x);
};