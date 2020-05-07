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
