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
	if (back.lock() == nullptr)
		front = instr;
	else {
		back.lock()->setNextInstr(instr);
		instr->setPreviousInstr(back.lock());
	}
	back = instr;
}

void BasicBlock::append_before_back(std::shared_ptr<IRInstruction> i)
{
	if (front == back.lock()) {
		i->setNextInstr(front);
		front = i;
	}
	else {
		i->setNextInstr(back.lock());
		i->setPreviousInstr(back.lock()->getPreviousInstr());
		i->getPreviousInstr()->setNextInstr(i);
	}
	back.lock()->setPreviousInstr(i);
}

void BasicBlock::remove_back()
{
	if (back.lock() == nullptr) return; //throw error?
	auto newBack = back.lock()->getPreviousInstr();
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
	else if (tag == IRInstruction::RET)
		func.lock()->appendReturnInstr(std::static_pointer_cast<Return>(instr));
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
	if (i->getPreviousInstr() == nullptr) i->getBlock()->setFront(i->getNextInstr());
	if (i->getNextInstr() == nullptr) i->getBlock()->setBack(i->getPreviousInstr());
	i->removeThis();
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
