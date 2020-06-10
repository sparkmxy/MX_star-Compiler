#include "cfg_pass.h"


void CFG_Pass::resolveDefineUseChain(std::shared_ptr<Function> f)
{
	use.clear();
	def.clear();
	auto &blocks = f->getBlockList();
	for (auto &block : blocks) {
		for (auto i = block->getFront(); i != nullptr; i = i->getNextInstr()) {
			if (i->getDefReg() != nullptr)
				def[i->getDefReg()] = i;
			if (i->getTag() == IRInstruction::PHI) {
				auto options = std::static_pointer_cast<PhiFunction>(i)->getRelatedRegs();
				for (auto &p : options) use[p.first].insert(i);
			}
			else {
				auto regs = i->getUseRegs();
				for (auto &reg : regs) use[reg].insert(i);
			}
		}
	}
}

void CFG_Pass::updateDTinfo(std::shared_ptr<Function> f)
{
	if (f == nullptr)
		for (auto &ff : ir->getFunctions())
			f->setDT(std::make_shared<DominatorTree>(f));
	else
		f->setDT(std::make_shared<DominatorTree>(f));
}

void CFG_Pass::computeRecursiveCalleeSet()
{
	calleeSet.clear();
	recursiveCalleeSet.clear();

	for (auto &f : ir->getFunctions())
		for (auto &b : f->getBlockList())
			for (auto i = b->getFront(); i != nullptr; i = i->getNextInstr())
				if (i->getTag() == IRInstruction::CALL)
					calleeSet[f].insert(std::static_pointer_cast<Call>(i)->getFunction());

	bool changed;
	do
	{
		changed = false;
		for (auto &f : ir->getFunctions()) {
			auto new_calleeSet = calleeSet[f];
			for (auto callee : calleeSet[f])
				for (auto ff : recursiveCalleeSet[callee]) new_calleeSet.insert(ff);

			if (new_calleeSet != recursiveCalleeSet[f]) {
				recursiveCalleeSet[f] = new_calleeSet;
				changed = true;
			}
		}
	} while (changed);
}