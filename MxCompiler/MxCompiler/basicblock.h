#pragma once

#include "pch.h"
#include "IRinstruction.h"
#include "Function.h"

/*
Class: BasicBlock
*/
class BasicBlock : public std::enable_shared_from_this<BasicBlock>{
public:
	enum Tag  // for debugging
	{
		FOR_BODY,
		TRUE, FALSE, FINAL,
		LHS_TRUE, LHS_FALSE
	};

	BasicBlock(std::shared_ptr<Function> _func, Tag _tag)
		:func(_func), tag(_tag), endFlag(false){}
	
	Tag getTag() { return tag; }
	std::shared_ptr<Function> getFunction() { return func; }

	void append_front(std::shared_ptr<IRInstruction> instr);
	void append_back(std::shared_ptr<IRInstruction> instr);
	void remove_back();
	
	void append_from(std::shared_ptr<BasicBlock> block);
	void append_to(std::shared_ptr<BasicBlock> block);
	void erase_from(std::shared_ptr<BasicBlock> block);
	void erase_to(std::shared_ptr<BasicBlock> block);
	void link_to_block(std::shared_ptr<BasicBlock> block);
	void endWith(std::shared_ptr<IRInstruction> instr);

	bool ended() { return endFlag; }
private:
	BlockTag tag;
	std::shared_ptr<Function> func;
	
	// instructions form a list
	std::shared_ptr<IRInstruction> front, back;
	
	std::set<std::shared_ptr<BasicBlock> > from, to;

	bool endFlag;
};