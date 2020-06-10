#include "functionInliner.h"

bool FunctionInliner::run()
{
	prepare();
	non_recursive();
	recursive();
	return true;
}

void FunctionInliner::prepare()
{
	call_cnt.clear();
	for (auto &f : ir->getFunctions()) {
		int cnt = 0;
		for(auto b : f->getBlockList())
			for (auto i = b->getFront(); i != nullptr; i = i->getNextInstr()) {
				cnt++;
				if (i->getTag() == IRInstruction::CALL) 
					call_cnt[std::static_pointer_cast<Call>(i)->getFunction()]++;
			}

		instr_cnt[f] = cnt;
	}
}

void FunctionInliner::non_recursive()
{
	bool changed;
	do
	{
		changed = false;
		for (auto &f : ir->getFunctions()) {
			for(auto &b : f->getBlockList())
				for (auto i = b->getFront(); i != nullptr;) {
					if (i->getTag() == IRInstruction::CALL) {
						auto call = std::static_pointer_cast<Call>(i);
						if (okForInlining(call->getFunction())) {
							changed = true;
							i = expand(call);
							instr_cnt[f] += instr_cnt[call->getFunction()];
							call_cnt[call->getFunction()]--;
						}
						else i = i->getNextInstr();
					}
					else i = i->getNextInstr();

				}
			updateDTinfo(f);
		}
	} while (changed);

	for (auto it : call_cnt)
		if (it.second == 0 && it.first->getName() != "__bootstrap")
			ir->omitFunction(it.first);
}

void FunctionInliner::recursive()
{
	bool changed;
	int depth = 0;
	do
	{
		changed = false;
		createShadows();

		for (auto &f : ir->getFunctions()) {
			for(auto &b : f->getBlockList())
				for (auto i = b->getFront(); i != nullptr;) {
					if (i->getTag() == IRInstruction::CALL) {
						auto call = std::static_pointer_cast<Call>(i);
						if (okForRecursivelyInlining(call->getFunction())) {
							changed = true;
							i = expand(call);
							instr_cnt[f] += instr_cnt[call->getFunction()];
						}
						else i = i->getNextInstr();
					}
					else i = i->getNextInstr();

				}
			updateDTinfo(f);
		}
		depth++;
	} while (changed && depth < MAX_INLINING_DEPTH);

	computeRecursiveCalleeSet();
}

std::shared_ptr<IRInstruction> FunctionInliner::expand(std::shared_ptr<Call> c)
{
	std::shared_ptr<Function> callee = shadows[c->getFunction()], caller = c->getBlock()->getFunction();
	auto calling_block = c->getBlock();

	auto splitter = std::make_shared<BasicBlock>(caller, BasicBlock::SPLITTER);
	for (auto to_block : calling_block->getBlocksTo()) to_block->replaceBlockTo(calling_block, splitter);

	std::vector<std::shared_ptr<IRInstruction> > afterCall;
	while (calling_block->getBack() != c) {
		afterCall.push_back(calling_block->getBack());
		calling_block->remove_back(); // be careful when removing back()
	}

	for (int j = afterCall.size() - 1; j >= 0; j--) {
		auto i = afterCall[j];
		i->setBlock(splitter);
		if (i->getTag() == IRInstruction::JUMP || i->getTag() == IRInstruction::BRANCH || i->getTag() || i->getTag() == IRInstruction::RET) {
			splitter->endWith(i);
		}
		else splitter->append_back(i);
	}
	splitter->getFront()->setNextInstr(nullptr);

	shadowBlocks.clear();
	globalVarInit();

	shadowBlocks[callee->getEntry()] = calling_block;
	shadowBlocks[callee->getExit()] = splitter;
	if (caller->getExit() == calling_block) caller->setExit(splitter);

	// copy argments
	for (auto arg : c->getArgs()) {
		auto new_arg = std::make_shared<VirtualReg>();
		appendInstrBefore(c, std::make_shared<Quadruple>(c->getBlock(), Quadruple::MOVE, new_arg, arg));
		shadowOperands[arg] = new_arg;
	}
	removeInstruction(c);

	for (auto &b : callee->getBlockList())
		if(shadowBlocks.find(b) == shadowBlocks.end())  // entry and exit are already set
			shadowBlocks[b] = std::make_shared<BasicBlock>(caller, BasicBlock::SHADOW);

	auto front_of_spilitter = splitter->getFront();
	for (auto &b : callee->getBlockList()) {
		for (auto i = b->getFront(); i != nullptr; i = i->getNextInstr()) {
			for (auto use : i->getUseRegs()) setShadowOperand(use);
			if (i->getDefReg() != nullptr) setShadowOperand(i->getDefReg());

			if (shadowBlocks[b] == splitter) {  // the exit of callee
				if (i != b->getBack())   // ignore the return instruction
					appendInstrBefore(front_of_spilitter, i->makeShadow(shadowBlocks, shadowOperands));
			}
			else {
				auto shadowInstr = i->makeShadow(shadowBlocks, shadowOperands);
				if (i == b->getBack())
					shadowBlocks[b]->endWith(shadowInstr);
				else shadowBlocks[b]->append_back(shadowInstr);
			}
		}
	}

	if (!calling_block->ended())
		calling_block->endWith(std::make_shared<Jump>(calling_block, splitter));

	// return
	auto ret = std::static_pointer_cast<Return>(callee->getExit()->getBack());
	if (ret->getValue() != nullptr) {
		std::shared_ptr<Operand> return_value = ret->getValue();
		if (shadowOperands.find(return_value) != shadowOperands.end()) return_value = shadowOperands[return_value];
		appendInstrBefore(front_of_spilitter, std::make_shared<Quadruple>(
			splitter, Quadruple::MOVE, c->getResult(), return_value));
	}
	return front_of_spilitter;
}

bool FunctionInliner::okForInlining(std::shared_ptr<Function> f)
{
	if (f->getObjRef() != nullptr) return false; // class method
	if (call_cnt[f] == 0) return false;
	return instr_cnt[f] <= INLINE_THRESHOLD;
}

bool FunctionInliner::okForRecursivelyInlining(std::shared_ptr<Function> f)
{
	if (recursiveCalleeSet[f].find(f) != recursiveCalleeSet[f].end()) return false; //recursive;
	return okForInlining(f);
}

void FunctionInliner::createShadows()
{
	shadows.clear();
	for (auto &f : ir->getFunctions())
		if (recursiveCalleeSet[f].find(f) != recursiveCalleeSet[f].end())
			shadows[f] = createShadowFunction(f);
		else shadows[f] = f;  // no need to create a shadow
}

std::shared_ptr<Function> FunctionInliner::createShadowFunction(std::shared_ptr<Function> f)
{
	auto shadow = std::make_shared<Function>("__shadow_" + f->getName(), nullptr); // is this ok?

	// no shadow blocks and registers now
	shadowBlocks.clear();
	shadowOperands.clear();

	auto blocks = f->getBlockList();
	for (auto &b : blocks) shadowBlocks[b] = std::make_shared<BasicBlock>(f, b->getTag());
	for (auto &b : blocks) {
		for (auto i = b->getFront(); i != nullptr; i = i->getNextInstr()) {
			if (i == b->getBack()) shadowBlocks[b]->endWith(i->makeShadow(shadowBlocks, shadowOperands));
			else shadowBlocks[b]->append_back(i->makeShadow(shadowBlocks, shadowOperands));
			//return instructions are not added to shadow function!
		}
	}
	shadow->setEntry(shadowBlocks[f->getEntry()]);
	shadow->setExit(shadowBlocks[f->getExit()]);
	shadow->setObjRef(f->getObjRef());
	shadow->getArgs() = f->getArgs();
	shadow->setDT(std::make_shared<DominatorTree>(shadow, true));
	return shadow;
}

void FunctionInliner::setShadowOperand(std::shared_ptr<Operand> op)
{
	if (shadowOperands.find(op) == shadowOperands.end())
		shadowOperands[op] = std::make_shared<VirtualReg>();
}

void FunctionInliner::globalVarInit()
{
	shadowOperands.clear();
	for (auto var : ir->getGlbVars()) {
		auto new_reg = std::make_shared<VirtualReg>();
		new_reg->markAsGlobal();
		shadowOperands[var] = new_reg;
	}
	for (auto str : ir->getStringConstants())
		shadowOperands[str->getReg()] = str->getReg();
}
