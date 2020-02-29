#pragma once

#include "pch.h"
#include "IRinstruction.h"

class Function;
/*
This struct hold the informations related to the dominance tree.
*/
struct DT_Info {
	std::vector<std::shared_ptr<BasicBlock> > DEdges, JEdges;
	int depth; // the depth in dominance tree
	int dfn;
};

/*
Class: BasicBlock
*/
class BasicBlock : public std::enable_shared_from_this<BasicBlock>{
public:
	enum Tag  // for debugging
	{
		FOR_BODY, FOR_COND, FOR_ITER, FOR_FINAL,
		WHILE_BODY, WHILE_COND, WHILE_FINAL,
		IF_TRUE, IF_FALSE, IF_FINAL,
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

	DT_Info &getDTInfo() { return dtInfo;}

	std::unordered_set<std::shared_ptr<BasicBlock> > &getBlocksTo() { return to; }
	std::unordered_set<std::shared_ptr<BasicBlock> > &getBlocksFrom() { return from; }
private:
	Tag tag;
	std::shared_ptr<Function> func;
	
	// instructions form a list
	std::shared_ptr<IRInstruction> front, back;
	
	std::unordered_set<std::shared_ptr<BasicBlock> > from, to;

	bool endFlag;

	// for SSA	
	DT_Info dtInfo;

};