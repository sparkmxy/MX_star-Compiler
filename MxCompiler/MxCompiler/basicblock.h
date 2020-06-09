#pragma once

#include "pch.h"
#include "IRinstruction.h"
#include "cfg_visitor.h"
#include "Function.h"

/*
This struct hold the informations related to the dominance tree.
*/
struct DT_Info {
	std::vector<std::shared_ptr<BasicBlock> > DEdges, JEdges;
	int dfn; // the dfn in the first place
	int depth; // the depth in dominance tree
	int dt_dfn, dt_dfn_r; // the subtree on DT is [dt_dfn, dt_dfn_r]
	std::shared_ptr<BasicBlock> idom, sdom;    // immediate dominator and semi-dominator

	void clear() {
		DEdges.clear();
		JEdges.clear();
		idom = sdom = nullptr;
	}
};

/*
Class: BasicBlock
*/

static const std::string namesForBasicBlocks[] = {
	"entry", "exit",
	"for_body", "for_cond", "for_iter", "for_final",
	"while_body", "while_cond", "while_final",
	"if_true", "if_false", "if_final",
	"true", "false", "final",
	"lhs_true", "lhs_false",
	"parallel_copy"
};

class BasicBlock : public std::enable_shared_from_this<BasicBlock>{
public:
	enum Tag  
	{
		ENTRY, EXIT,
		FOR_BODY, FOR_COND, FOR_ITER, FOR_FINAL,
		WHILE_BODY, WHILE_COND, WHILE_FINAL,
		IF_TRUE, IF_FALSE, IF_FINAL,
		TRUE, FALSE, FINAL,
		LHS_TRUE, LHS_FALSE,
		PARALLEL_COPY
	};

	BasicBlock(std::weak_ptr<Function> _func, Tag _tag)
		:func(_func), tag(_tag), endFlag(false){}
	
	Tag getTag() { return tag; }
	std::shared_ptr<Function> getFunction() { return func.lock(); }

	void append_front(std::shared_ptr<IRInstruction> instr);
	void append_back(std::shared_ptr<IRInstruction> instr);
	void append_before_back(std::shared_ptr<IRInstruction> i);
	void remove_back();

	std::shared_ptr<IRInstruction> getFront() { return front; }
	void setFront(std::shared_ptr<IRInstruction> i) { front = i; }

	std::shared_ptr<IRInstruction> getBack() { return back.lock(); }
	void setBack(std::shared_ptr<IRInstruction> i) { back = i; }

	void append_from(std::shared_ptr<BasicBlock> block);
	void append_to(std::shared_ptr<BasicBlock> block);
	void erase_from(std::shared_ptr<BasicBlock> block);
	void erase_to(std::shared_ptr<BasicBlock> block);
	void link_to_block(std::shared_ptr<BasicBlock> block);
	void endWith(std::shared_ptr<IRInstruction> instr);

	bool ended() { return endFlag; }

	DT_Info &getDTInfo() { return dtInfo;}
	const DT_Info &getDTInfo() const { return dtInfo; }
	void clearDTInfo() { dtInfo.clear(); }

	std::set<std::shared_ptr<BasicBlock> > &getBlocksTo() { return to; }
	std::set<std::shared_ptr<BasicBlock> > &getBlocksFrom() { return from; }

	std::string toString() { return namesForBasicBlocks[tag]; }

	void destroyEdges();

	ACCEPT_CFG_VISITOR

private:
	Tag tag;
	std::weak_ptr<Function> func;
	
	// instructions form a list
	std::shared_ptr<IRInstruction> front;
	std::weak_ptr<IRInstruction> back;
	
	std::set<std::shared_ptr<BasicBlock> > from, to;

	bool endFlag;

	// for SSA	
	DT_Info dtInfo;

};

void replaceInstruction(std::shared_ptr<IRInstruction> old, std::shared_ptr<IRInstruction> _new);
void removeInstruction(std::shared_ptr<IRInstruction> i);
void appendInstrBefore(std::shared_ptr<IRInstruction> i, std::shared_ptr<IRInstruction> new_i);
void appendInstrAfter(std::shared_ptr<IRInstruction> i, std::shared_ptr<IRInstruction> new_i);