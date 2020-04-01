#pragma once

#include "pch.h"
#include "IRinstruction.h"

class DominanceTree;
class BasicBlock;
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

	std::shared_ptr<DominanceTree> getDT() { return dt; }
	void initDT(std::shared_ptr<DominanceTree> _dt) { dt = _dt; }
	void append_var(std::shared_ptr<VirtualReg> reg) { vars.emplace_back(reg); }
	std::vector<std::shared_ptr<VirtualReg> > &getVars() { return vars; }
private:
	std::shared_ptr<BasicBlock> entry, exit;
	std::string name;

	std::vector<std::shared_ptr<Return> > retInstrs;
	std::vector<std::shared_ptr<Register> > args;

	std::shared_ptr<Operand> objRef;

	//for SSA
	std::shared_ptr<DominanceTree> dt;
	//return a list of block in this function module in DFS order on dominance tree;
	std::vector<std::shared_ptr<BasicBlock> > blocks;
	std::vector<std::shared_ptr<VirtualReg> > vars;
};