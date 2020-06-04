#include "dominance.h"

bool DominatorTree::isDominating(std::shared_ptr<BasicBlock> x, std::shared_ptr<BasicBlock> y)
{
	auto &x_info = x->getDTInfo();
	int y_dfn = y->getDTInfo().dt_dfn;
	return x_info.dt_dfn <= y_dfn && y_dfn <= x_info.dt_dfn_r;
}

void DominatorTree::DFS(std::shared_ptr<BasicBlock> x, std::shared_ptr<BasicBlock> father, int dep)
{
	visited.insert(x);
	x->clearDTInfo();
	x->getDTInfo().dfn = dfs_clock++;
	x->getDTInfo().depth = dep;
	fa.push_back(father);
	idfn.push_back(x);
	x->getDTInfo().sdom = x;  // initialize sdom
	x->getDTInfo().idom = f->getEntry();
	auto &edges = x->getBlocksTo();
	for (auto &y : edges) 
		if(visited.find(y) == visited.end())
			DFS(y, x, dep + 1);
}

void DominatorTree::workOutIdoms()
{
	for (int i = idfn.size() - 1; i > 1; i--) {
		auto x = idfn[i];
		auto &from_blocks = x->getBlocksFrom();
		for (auto &y : from_blocks) {
			if (y->getBlocksFrom().empty()) continue;  // non-reachable
			int j = y->getDTInfo().dfn;
			find(j);
			auto z = idfn[val[j]]->getDTInfo().sdom;
			if (z->getDTInfo().dfn < x->getDTInfo().sdom->getDTInfo().dfn)  
				x->getDTInfo().sdom = z;
		}
		S[x->getDTInfo().dfn] = fa[i]->getDTInfo().dfn;   // link this node to its parent
		sdomEqcls[x->getDTInfo().sdom].push_back(x);
		for (auto &v : sdomEqcls[fa[i]]) {
			int v_dfn = v->getDTInfo().dfn;
			find(v_dfn);
			if (fa[i] == idfn[val[v_dfn]]->getDTInfo().sdom)
				v->getDTInfo().idom = fa[i];
			else v->getDTInfo().idom = idfn[val[v_dfn]];
		}
		sdomEqcls[fa[i]].clear();
	}
	for (int i = 1; i < idfn.size(); i++) {
		auto x = idfn[i];

		if (x->getDTInfo().sdom != x->getDTInfo().idom)
			x->getDTInfo().idom = x->getDTInfo().idom->getDTInfo().idom;
		x->getDTInfo().idom->getDTInfo().DEdges.push_back(x);
	}
}

void DominatorTree::buildDJGraph(std::shared_ptr<BasicBlock> x)
{
	x->getDTInfo().dt_dfn = dfs_clock++;
	x->getDTInfo().depth = x->getDTInfo().idom->getDTInfo().depth + 1;
	f->appendBlocktoList(x);
	auto Dedges = x->getDTInfo().DEdges;
	for (auto &y : Dedges) buildDJGraph(y);
	x->getDTInfo().dt_dfn_r = dfs_clock;

	// find out J-edges
	auto &edges = x->getBlocksTo();
	for(auto &y : edges)
		if (!isStrictlyDominating(x,y)) {  // this is a J-edge
			x->getDTInfo().JEdges.push_back(y);
		}


}

bool DominatorTree::isStrictlyDominating(std::shared_ptr<BasicBlock> x, std::shared_ptr<BasicBlock> y)
{
	return x != y && isDominating(x, y);
}


void DominatorTree::union_find_init(int n)
{
	val.resize(n);
	S.resize(n);
	for (int i = 0; i < n; i++) S[i] = val[i] = i;
}

// The index of S and val is dfn and val stores dfn
int DominatorTree::find(int x)
{
	if (S[x] == x) return x;
	int r = find(S[x]);
	if (idfn[val[S[x]]]->getDTInfo().sdom->getDTInfo().dfn < idfn[val[x]]->getDTInfo().sdom->getDTInfo().dfn)
		val[x] = val[S[x]];
	return S[x] = r;
}
