#include "SSADestructor.h"
#include <queue>

bool SSADestructor::run()
{
	auto functions = ir->getFunctions();
	for (auto &f : functions) {
		removePhiFunction(f);
		f->setDT(std::make_shared<DominatorTree>(f));
		sequentializeParalleCopy(f);
	}
	ir->setSSAflag(false);
	return true;
}

void SSADestructor::removePhiFunction(std::shared_ptr<Function> f)
{   
	auto &blocks = f->getBlockList();
	for (auto &b : blocks) {
		auto edges = b->getBlocksFrom();
		std::map<std::shared_ptr<BasicBlock>, std::shared_ptr<BasicBlock>> whereToPut;
		for (auto &B : edges) { // B -> b
			if (B->getBlocksTo().size() > 1) {
				auto new_block = std::make_shared<BasicBlock>(f, BasicBlock::PARALLEL_COPY);
				new_block->endWith(std::make_shared<Jump>(new_block, b));
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
			for (auto &p : options) { 
				std::shared_ptr<Operand> src;
				if (p.first == nullptr) src = std::make_shared<Immediate>(0);
				else src = p.first;
				parallel_copy[whereToPut[p.second.lock()]].insert(std::make_shared<ParallelCopy>(i->getDst(), src));
			}
			removeInstruction(i);
		}
	}
}

void SSADestructor::sequentializeParalleCopy(std::shared_ptr<Function> f)
{
	f->setDT(std::make_shared<DominatorTree>(f)); // renew blocklist
	auto &blocks = f->getBlockList();
	for (auto &b : blocks) {
		// something like topsort
		auto &copies = parallel_copy[b];
		std::queue<std::shared_ptr<ParallelCopy> > Q;

		std::map<std::shared_ptr<Operand>, int> degree;
		std::map<std::shared_ptr<Operand>, std::shared_ptr<ParallelCopy> > dstToCopy;

		auto new_reg = std::make_shared<VirtualReg>(Operand::REG_VAL, "seq");
		std::set<std::shared_ptr<Operand> > breakers;

		for (auto &c : copies) degree[c->src]++;
		for (auto &c : copies) {
			if (degree[c->dst] == 0) Q.push(c);
			dstToCopy[c->dst] = c;
		}

		while (!copies.empty()) {
			while (!Q.empty()) {
				auto c = Q.front();
				Q.pop();
				if (c->src == c->dst) continue;

				auto src = breakers.find(c->src) == breakers.end() ? c->src : new_reg;
				b->append_before_back(std::make_shared<Quadruple>(b, Quadruple::MOVE, c->dst, src));
				
				degree[src]--;
				if (src != new_reg && degree[src] == 0 && dstToCopy[src] != nullptr) 
					Q.push(dstToCopy[c->src]);
				copies.erase(c);
			}
			// break a cycle
			if (!copies.empty()) {
				auto c = *copies.begin();
				if (c->src != c->dst) {
					b->append_before_back(std::make_shared<Quadruple>(b, Quadruple::MOVE, new_reg, c->src));
					breakers.insert(c->src);
					if (--degree[c->src] == 0 && dstToCopy[c->src] != nullptr)
						Q.push(dstToCopy[c->src]);
				}
				else copies.erase(c);
			}
		}
	}
}
