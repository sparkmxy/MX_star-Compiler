#pragma once

#include "pch.h"
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

private:
	std::shared_ptr<BasicBlock> entry, exit;
	std::string name;

	std::vector<std::shared_ptr<Return> > retInstrs;
	std::vector<std::shared_ptr<Register> > args;
};