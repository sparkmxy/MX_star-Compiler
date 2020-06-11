#include "CFGCleanUp.h"

bool CFGCleanUpPass::run()
{
	changed = false;
	for (auto &f : ir->getFunctions()) {
		rewriteTrivialBranches(f);
		convertMultiplyByConst(f);
		if (!ir->isSSA()) {
			omitTrivialBlocks(f);
			mergeBlockChain(f);
		}
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

void CFGCleanUpPass::convertMultiplyByConst(std::shared_ptr<Function> f) {
	for(auto &b: f->getBlockList())
		for (auto i = b->getFront(); i != nullptr; i = i->getNextInstr())
			if (i->getTag() == IRInstruction::QUADR) {
				auto q = std::static_pointer_cast<Quadruple>(i);
				std::shared_ptr<Operand> src1 = q->getSrc1(), src2 = q->getSrc2();
				if(q->getOp() == Quadruple::TIMES && 
					(src1->category() == Operand::IMM || src2->category() == Operand::IMM)){
					if (src2->category() != Operand::IMM) std::swap(src1, src2);
					int x = std::static_pointer_cast<Immediate>(src2)->getValue();
					if ((1 << _log2(x)) == x) {
						changed = true;
						replaceInstruction(i, std::make_shared<Quadruple>(i->getBlock(),
							Quadruple::LSHIFT, q->getDst(), src1, std::make_shared<Immediate>(_log2(x))));
					}
				}
			}
}

void CFGCleanUpPass::mergeBlockChain(std::shared_ptr<Function> f)
{
	auto blocks = f->getBlockList();
	for (int i = blocks.size() - 1; i >= 0; i--) {
		auto b = blocks[i];
		if (b->getBlocksTo().size() == 1) {
			auto to_block = *blocks[i]->getBlocksTo().begin();
			if (to_block != f->getEntry() && to_block->getBlocksFrom().size() == 1 && to_block != b) {
				changed = true;
				// merge to_block to  b
				for (auto &bb : to_block->getBlocksTo()) bb->replaceBlockFrom(to_block, b);
				b->remove_back();
				b->append_back(to_block->getFront());
				b->setBack(to_block->getBack());
				for (auto i = to_block->getFront(); i != nullptr; i = i->getNextInstr()) i->setBlock(b);

				if (to_block == f->getExit()) f->setExit(b);
			}
		}
	}
}
