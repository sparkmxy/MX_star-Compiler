#include "dominance.h"

bool DominanceTree::isDominating(std::shared_ptr<BasicBlock> x, std::shared_ptr<BasicBlock> y)
{
	auto &x_info = x->getDTInfo();
	int y_dfn = y->getDTInfo().dt_dfn;
	return x_info.dt_dfn <= y_dfn && y_dfn <= x_info.dt_dfn_r;
}

void DominanceTree::DFS(std::shared_ptr<BasicBlock> x, std::shared_ptr<BasicBlock> father, int dep)
{
	visited.insert(x);
	x->getDTInfo().dfn = dfs_clock++;
	x->getDTInfo().depth = dep;
	fa.push_back(father);
	idfn.push_back(x);
	auto &edges = x->getBlocksTo();
	for (auto &y : edges) 
		if(visited.find(y) == visited.end())
			DFS(y, x, dep + 1);
}

void DominanceTree::workOutIdoms()
{
	for (int i = idfn.size() - 1; i > 1; i--) {
		auto x = idfn[i];
		auto &from_blocks = x->getBlocksFrom();
		for (auto &y : from_blocks) {
			int j = y->getDTInfo().dfn;
			find(j);
			auto z = idfn[val[j]]->getDTInfo().sdom;
			if (z->getDTInfo().dt_dfn < x->getDTInfo().sdom->getDTInfo().dfn)
				x->getDTInfo().sdom = z;
		}
		S[x->getDTInfo().dfn] = fa[i]->getDTInfo().dfn;
		sdomEqcls[x->getDTInfo().sdom].push_back(x);
		for (auto &v : sdomEqcls[fa[i]]) {
			int v_dfn = v->getDTInfo().dfn;
			find(v_dfn);
			if (v->getDTInfo().sdom == idfn[val[v_dfn]]->getDTInfo().sdom)
				v->getDTInfo().idom = idfn[val[v_dfn]]->getDTInfo().sdom;
			else v->getDTInfo().idom = idfn[val[v_dfn]];
		}
		sdomEqcls[fa[i]].clear();
	}
	for (int i = 1; i < idfn.size(); i++) {
		auto x = idfn[i];
		if (x->getDTInfo().sdom != x->getDTInfo().idom)
			x->getDTInfo().idom = x->getDTInfo().idom->getDTInfo().idom;
	}
}

void DominanceTree::buildDominaceTree(std::shared_ptr<BasicBlock> x)
{
	x->getDTInfo().dt_dfn = dfs_clock++;
	auto &edges = x->getBlocksTo();
	for(auto &y : edges)
		if (y->getDTInfo().idom != x) {  // this is a J-edge
			x->getDTInfo().JEdges.push_back(y);
		}
		else {
			x->getDTInfo().DEdges.push_back(y);
			buildDominaceTree(y);
		}
	x->getDTInfo().dt_dfn_r = dfs_clock;
}

void DominanceTree::union_find_init(int n)
{
	val.resize(n);
	S.resize(n);
	for (int i = 0; i < n; i++) S[i] = i;
}


int DominanceTree::find(int x)
{
	if (S[x] == x) return x;
	int r = find(S[x]);
	if (idfn[val[S[x]]]->getDTInfo().sdom->getDTInfo().dfn < idfn[val[x]]->getDTInfo().sdom->getDTInfo().dfn)
		val[x] = val[S[x]];
	return S[x] = r;
}
