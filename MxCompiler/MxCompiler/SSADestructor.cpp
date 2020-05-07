#include "SSADestructor.h"

bool SSADestructor::run()
{
	auto functions = ir->getFunctions();
	for (auto &f : functions) {
		removePhiFunction(f);
		f->setDT(std::make_shared<DominatorTree>(f));
		replaceParalleCopy(f);
	}
	return true;
}

void SSADestructor::removePhiFunction(std::shared_ptr<Function> f)
{   
	auto &blocks = f->getBlockList();
	for (auto &b : blocks) {
		auto edges = b->getBlocksFrom();
		std::unordered_map<std::shared_ptr<BasicBlock>, std::shared_ptr<BasicBlock>> whereToPut;
		for (auto &B : edges) { // B -> b
			if (B->getBlocksTo().size() > 1) {
				auto new_block = std::make_shared<BasicBlock>(f, BasicBlock::PARALLEL_COPY);
				B->erase_to(b);
				B->append_to(new_block);
				new_block->append_from(B);
				new_block->append_to(b);
				b->erase_from(B);
				b->append_from(new_block);
				std::static_pointer_cast<Branch>(B->getBack())->replaceTargetBlock(b, new_block);
				whereToPut[B] = new_block;
			}
			else whereToPut[B] = B;
		}

		while (b->getFront()->getTag() == IRInstruction::PHI) {
			auto i = std::static_pointer_cast<PhiFunction>(b->getFront());
			auto options = i->getRelatedRegs();
			for (auto &p : options)
				parallel_copy[whereToPut[p.second]].insert(
					ParallelCopy(std::static_pointer_cast<VirtualReg>(i->getDst()), p.first));
			removeInstruction(i);
		}
	}
}

void SSADestructor::replaceParalleCopy(std::shared_ptr<Function> f)
{

}
