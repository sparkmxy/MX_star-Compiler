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
			auto regs = i->getUseRegs();
			for (auto &reg : regs) use[reg].insert(i);
		}
	}
}
