#pragma once

#include "pch.h"
#include "IRinstruction.h"
#include "basicblock.h"

/*
Class: Function
The main component of a function module is its control flow graph. 
*/
class Function {
public:
	Function(const std::string &_name) : name(_name) {}

	void appendReturnInstr(std::shared_ptr<Return> ret);
	void appendArg(std::shared_ptr<Register> arg);

	// getters/setters
	std::string getName() { return name; }

	std::shared_ptr<BasicBlock> getEntry() { return entry; }
	void setEntry(const std::shared_ptr<BasicBlock> &_entry) { entry = _entry; }

	std::shared_ptr<BasicBlock> getExit() { return exit; }
	void setExit(const std::shared_ptr<BasicBlock> &_exit) { exit = _exit; }

	std::shared_ptr<Operand> getObjRef() { return objRef; }
	void setObjRef(const std::shared_ptr<Operand> &ref) { objRef = ref; }

	std::vector<std::shared_ptr<BasicBlock> > &getBlockList() { return blocks; }
	//for SSA
	void buildDominanceTree();
private:
	std::shared_ptr<BasicBlock> entry, exit;
	std::string name;

	std::vector<std::shared_ptr<Return> > retInstrs;
	std::vector<std::shared_ptr<Register> > args;

	std::shared_ptr<Operand> objRef;

	//for SSA
	std::vector<std::shared_ptr<BasicBlock> > blocks;

	void DFS(std::shared_ptr<BasicBlock> blk,int dep = 0);
	std::unordered_set<std::shared_ptr<BasicBlock> > visited;
	int dfs_clock;
};