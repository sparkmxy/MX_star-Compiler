#include "basicblock.h"

void BasicBlock::append_front(std::shared_ptr<IRInstruction> instr)
{
	if (front == nullptr) front = back = instr;
	else {
		front->setPreviousInstr(instr);
		instr->setNextInstr(front);
		front = instr;
	}
}

void BasicBlock::append_back(std::shared_ptr<IRInstruction> instr)
{
	if (back == nullptr) front = back = instr;
	else {
		back->setNextInstr(instr);
		instr->setPreviousInstr(back);
		back = instr;
	}
}

void BasicBlock::remove_back()
{
	if (back == nullptr) return; //throw error?
	auto newBack = back->getPreviousInstr();
	if (newBack == nullptr) front = back = nullptr;
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
		func->appendReturnInstr(std::static_pointer_cast<Return>(instr));
	endFlag = true;
}
