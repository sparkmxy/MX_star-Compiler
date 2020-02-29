#include "Function.h"

void Function::appendReturnInstr(std::shared_ptr<Return> ret)
{
	retInstrs.emplace_back(ret);
}

void Function::appendArg(std::shared_ptr<Register> arg)
{
	args.emplace_back(arg);
}

void Function::buildDominanceTree()
{
	visited.clear();
	dfs_clock = 0;
	DFS(entry);
}

void Function::DFS(std::shared_ptr<BasicBlock> blk,int dep)
{
	visited.insert(blk);
	blocks.push_back(blk);
	auto to_blocks = blk->getBlocksTo();
	auto &info = blk->getDTInfo();
	info.depth = dep;
	info.dfn = dfs_clock++;
	for (auto to_block : to_blocks)
		if (visited.find(to_block) == visited.end()) {  // this is a D-edge
			info.DEdges.push_back(to_block);
			DFS(to_block, dep + 1);
		}
		else {  // this is a J-edge
			info.JEdges.push_back(to_block);
		}
}
