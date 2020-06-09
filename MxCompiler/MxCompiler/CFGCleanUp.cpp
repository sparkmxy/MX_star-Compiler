#include "CFGCleanUp.h"

bool CFGCleanUpPass::run()
{
	changed = false;
	for (auto &f : ir->getFunctions()) {
		rewriteTrivialBranches(f);
		if(!ir->isSSA()) omitTrivialBlocks(f);
	}
	return changed;
}

void CFGCleanUpPass::rewriteTrivialBranches(std::shared_ptr<Function> f){
	for (auto &b : f->getBlockList()) 
		if (b->getBack()->getTag() == IRInstruction::BRANCH) {
			auto br = std::static_pointer_cast<Branch>(b->getBack());
			if (br->getTrueBlock() == br->getFalseBlock()) {
				changed = true;
				replaceInstruction(br, std::make_shared<Jump>(b, br->getTrueBlock()));
			}
			else if (br->getCondition()->category() == Operand::IMM) {
				changed = true;
				int cond = std::static_pointer_cast<Immediate>(br->getCondition())->getValue();
				std::shared_ptr<BasicBlock> target = br->getTrueBlock(), other = br->getFalseBlock();
				if (!cond) std::swap(target, other);

				replaceInstruction(br, std::make_shared<Jump>(b, target));
				
				for(auto i = other->getFront(); i->getTag() == IRInstruction::PHI; i = i->getNextInstr())
					std::static_pointer_cast<PhiFunction>(i)->removeOption(b);
			
				b->getBlocksTo().erase(other);
				other->getBlocksFrom().erase(b);
			}
		}
	updateDTinfo(f);
}


void CFGCleanUpPass::omitTrivialBlocks(std::shared_ptr<Function> f)
{
	for (auto &b : f->getBlockList()) 
		if (b != f->getEntry() &&
			b->getFront() == b->getBack() && b->getFront()->getTag() == IRInstruction::JUMP) {
			changed = true;

			auto j = std::static_pointer_cast<Jump>(b->getFront());
			auto target = j->getTarget();
			for (auto p : b->getBlocksFrom()) {
				p->getBlocksTo().erase(b);
				p->getBlocksTo().insert(target);
				target->getBlocksFrom().insert(p);

				if (p->getBack()->getTag() == IRInstruction::JUMP)
					std::static_pointer_cast<Jump>(p->getBack())->setTarget(target);
				else if (p->getBack()->getTag() == IRInstruction::BRANCH)
					std::static_pointer_cast<Branch>(p->getBack())->replaceTargetBlock(b, target);
			}
		}
	updateDTinfo(f);
}
