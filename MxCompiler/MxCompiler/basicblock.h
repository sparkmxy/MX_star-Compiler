#pragma once

#include "pch.h"
#include "IRinstruction.h"


class Function;

/*
Class: BasicBlock
*/
class BasicBlock {
public:
	BasicBlock(std::shared_ptr<Function> _func, const std::string &_name)
		:func(_func), name(_name) {}
	
	std::string getName() { return name; }
	std::shared_ptr<Function> getFunction() { return func; }

	void append_front(std::shared_ptr<IRInstruction> instr);
	void remove_back();
	
	void append_from(std::shared_ptr<BasicBlock> block);
	void append_to(std::shared_ptr<BasicBlock> block);
	
private:
	std::string name;
	std::shared_ptr<Function> func;
	
	// instructions form a list
	std::shared_ptr<IRInstruction> front, back;
	
	std::set<std::shared_ptr<BasicBlock> > from, to;
};