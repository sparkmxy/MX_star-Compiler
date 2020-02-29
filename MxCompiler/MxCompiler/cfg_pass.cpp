#include "cfg_pass.h"

/*
Note: 
This function run DFS method for each function module, 
and disdinguish D-edges and J-edges for later DF+ construction.
Moreover, a list of blocks a available for each function module after this proccess,
which is provided by <getBlockList> fucntion.
*/
void CFG_Pass::buildDominanceTree()
{
	auto functions = ir->getFunctions();
	for (auto &function : functions)
		function->buildDominanceTree();
}
