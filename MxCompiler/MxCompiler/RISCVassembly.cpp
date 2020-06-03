#include "RISCVassembly.h"

const std::vector<std::string> RISCVConfig::physicalRegNames = { 
		"zero", "ra", "sp", "gp", "tp", 
		"t0", "t1", "t2", "s0", "s1",
		"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
		"s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",
		"t3", "t4", "t5", "t6" 
};

const std::vector<std::string>  RISCVConfig::calleeSaveRegNames = {
	"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11" 
};

const std::vector<std::string>  RISCVConfig::callerSaveRegNames = {
	"ra", 
	"t0", "t1", "t2", 
	"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", 
	"t3","t4","t5","t6"   // temporaries
};

const std::vector<std::string> RISCVConfig::allocatableRegNames = {
		"ra", 
		"t0", "t1", "t2", "s0", "s1",
		"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
		"s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",
		"t3", "t4", "t5", "t6"
};

void RISCVBasicBlock::append(const std::shared_ptr<RISCVinstruction>& i)
{
	if (front == nullptr) {
		front = i;
		back = i;
	}
	else {
		i->setPrevInstr(back.lock());
		back.lock()->setNextInstr(i);
		back = i;
	}
	if (i->category() == RISCVinstruction::JUMP) {
		auto target = std::static_pointer_cast<JumpAssembly>(i)->getTarget();
		to.insert(target);
		target->insertFromBlock(shared_from_this());
	}
	else if (i->category() == RISCVinstruction::BTYPE) {
		auto target = std::static_pointer_cast<B_type>(i)->getTargetBlock();
		to.insert(target);
		target->insertFromBlock(shared_from_this());
	}
}

RISCVProgram::RISCVProgram()
{
	for (auto &name : RISCVConfig::physicalRegNames)
		physicalRegs[name] = std::make_shared<PhysicalRegister>(name);

	mallocFunc = std::make_shared<RISCVFunction>("malloc");
}


void RISCVProgram::resetPrecoloredRegs()
{
	for (auto it : physicalRegs) {
		it.second->info().clear();
		it.second->info().degree = 2333;
		it.second->info().color = it.second;
	}
}

std::unordered_set<std::shared_ptr<PhysicalRegister>> RISCVProgram::getAllocatableRegs()
{
	std::unordered_set<std::shared_ptr<PhysicalRegister>> ret;
	for (auto regName : RISCVConfig::allocatableRegNames)
		ret.insert(physicalRegs[regName]);
	return ret;
}

void removeRISCVinstruction(std::shared_ptr<RISCVinstruction> i)
{
	auto b = i->getBlock();

	if (b->getBack() == i) b->setBack(i->getPrevInstr());
	else i->getNextInstr()->setPrevInstr(i->getPrevInstr());

	if (b->getFront() == i) b->setFront(i->getNextInstr());
	else i->getPrevInstr()->setNextInstr(i->getNextInstr());
}

void appendBefore(std::shared_ptr<RISCVinstruction> i, std::shared_ptr<RISCVinstruction> new_i)
{
	auto b = i->getBlock();
	new_i->setNextInstr(i);
	new_i->setPrevInstr(i->getPrevInstr());
	if (i->getPrevInstr() != nullptr) i->getPrevInstr()->setNextInstr(new_i);
	i->setPrevInstr(new_i);
	if (i == b->getFront()) b->setFront(new_i);
}

void appendAfter(std::shared_ptr<RISCVinstruction> i, std::shared_ptr<RISCVinstruction> new_i)
{
	auto b = i->getBlock();
	new_i->setPrevInstr(i);
	new_i->setNextInstr(i->getNextInstr());
	if (i->getNextInstr() != nullptr) i->getNextInstr()->setPrevInstr(new_i);
	i->setNextInstr(new_i);
	if (i == b->getBack()) b->setBack(new_i);
}

int RISCVFunction::stackLocationFromBottom(int size)
{
	stackSizeFromBottom += size;
	return stackSizeFromBottom;
}

void RISCVFunction::computePreOrderList()
{
	blocks.clear();
	std::unordered_set<std::shared_ptr<RISCVBasicBlock> > visited;
	DFS(entry, visited);
}

void RISCVFunction::DFS(std::shared_ptr<RISCVBasicBlock> b, std::unordered_set<std::shared_ptr<RISCVBasicBlock>> &visited)
{
	if (visited.find(b) != visited.end()) return;
	blocks.push_back(b);
	visited.insert(b);
	for (auto bb : b->getToBlocks()) DFS(bb, visited);
}
