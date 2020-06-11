#include "basicblock.h"

void BasicBlock::append_front(std::shared_ptr<IRInstruction> instr)
{
	if (front == nullptr) {
		front = instr;
		back = instr;
	}
	else {
		front->setPreviousInstr(instr);
		instr->setNextInstr(front);
		front = instr;
	}
}

void BasicBlock::append_back(std::shared_ptr<IRInstruction> instr)
{
	if (back == nullptr)
		front = instr;
	else {
		back->setNextInstr(instr);
		instr->setPreviousInstr(back);
	}
	back = instr;
}

void BasicBlock::append_before_back(std::shared_ptr<IRInstruction> i)
{
	if (front == back) {
		i->setNextInstr(front);
		front = i;
	}
	else {
		i->setNextInstr(back);
		i->setPreviousInstr(back->getPreviousInstr());
		i->getPreviousInstr()->setNextInstr(i);
	}
	back->setPreviousInstr(i);
}

void BasicBlock::remove_back()
{
	if (back == nullptr) throw Error("poor mxy!");

	endFlag = false;
	if (back->getTag() == IRInstruction::JUMP) 
		erase_to(std::static_pointer_cast<Jump>(back)->getTarget());
	else if (back->getTag() == IRInstruction::BRANCH) {
		erase_to(std::static_pointer_cast<Branch>(back)->getTrueBlock());
		erase_to(std::static_pointer_cast<Branch>(back)->getFalseBlock());
	}
	// maintain the list
	auto newBack = back->getPreviousInstr();
	if (newBack == nullptr) {
		front = nullptr;
		back.reset();
	}
	else {
		newBack->setNextInstr(nullptr);
		back = newBack;
	}
}

void BasicBlock::append_from(std::shared_ptr<BasicBlock> block)
{
	from.insert(block);
}

void BasicBlock::append_to(std::shared_ptr<BasicBlock> block)
{
	to.insert(block);
}

void BasicBlock::erase_from(std::shared_ptr<BasicBlock> block)
{
	if (block != nullptr) from.erase(block);
}

void BasicBlock::erase_to(std::shared_ptr<BasicBlock> block)
{
	if (block != nullptr) to.erase(block);
}

void BasicBlock::link_to_block(std::shared_ptr<BasicBlock> block)
{
	if (block == nullptr) return;
	this->append_to(block);
	block->append_from(shared_from_this());
}

void BasicBlock::replaceBlockTo(std::shared_ptr<BasicBlock> b, std::shared_ptr<BasicBlock> new_block)
{
	if (to.find(b) == to.end()) throw Error("you are tiehanhan");
	to.erase(b);
	to.insert(new_block);
}

void BasicBlock::replaceBlockFrom(std::shared_ptr<BasicBlock> b, std::shared_ptr<BasicBlock> new_block)
{
	if (from.find(b) == from.end()) throw Error("you are a tiehanhan");
	from.erase(b);
	from.insert(new_block);
}

void BasicBlock::endWith(std::shared_ptr<IRInstruction> instr)
{
	auto tag = instr->getTag();
	append_back(instr);
	if (tag == IRInstruction::BRANCH) {
		auto branch = std::static_pointer_cast<Branch>(instr);
		link_to_block(branch->getTrueBlock());
		link_to_block(branch->getFalseBlock());
	}
	else if (tag == IRInstruction::JUMP)
		link_to_block(std::static_pointer_cast<Jump>(instr)->getTarget());
	endFlag = true;
}

void BasicBlock::destroyEdges()
{
	to.clear();
	from.clear();
	dtInfo.clear();
}

void replaceInstruction(std::shared_ptr<IRInstruction> old, std::shared_ptr<IRInstruction> _new)
{
	if (old->getPreviousInstr() == nullptr) old->getBlock()->setFront(_new);
	if (old->getNextInstr() == nullptr) old->getBlock()->setBack(_new);
	old->replaceBy(_new);
}

void removeInstruction(std::shared_ptr<IRInstruction> i)
{
	auto b = i->getBlock();
	if (b->getBack() == i) b->setBack(i->getPreviousInstr());
	else i->getNextInstr()->setPreviousInstr(i->getPreviousInstr());

	if (b->getFront() == i) b->setFront(i->getNextInstr());
	else i->getPreviousInstr()->setNextInstr(i->getNextInstr());
}

void appendInstrBefore(std::shared_ptr<IRInstruction> i, std::shared_ptr<IRInstruction> new_i)
{
	auto b = i->getBlock();
	new_i->setNextInstr(i);
	new_i->setPreviousInstr(i->getPreviousInstr());
	if (i->getPreviousInstr() != nullptr) i->getPreviousInstr()->setNextInstr(new_i);
	i->setPreviousInstr(new_i);
	if (i == b->getFront()) b->setFront(new_i);
}

void appendInstrAfter(std::shared_ptr<IRInstruction> i, std::shared_ptr<IRInstruction> new_i)
{
	auto b = i->getBlock();
	new_i->setPreviousInstr(i);
	new_i->setNextInstr(i->getNextInstr());
	if (i->getNextInstr() != nullptr) i->getNextInstr()->setPreviousInstr(new_i);
	i->setNextInstr(new_i);
	if (i == b->getBack()) b->setBack(new_i);
}
