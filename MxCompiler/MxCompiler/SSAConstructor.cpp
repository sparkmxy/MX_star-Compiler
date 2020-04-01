#include "SSAConstructor.h"

void SSAConstructor::constructSSA()
{
	initDT();
	collectVariales();
	insertPhiFunction();
	renameVariables();
}

void SSAConstructor::initDT()
{
	auto &functions = ir->getFunctions();
	for (auto function : functions) 
		function->initDT(std::make_shared<DominanceTree>(function));
}

void SSAConstructor::insertPhiFunction()
{
	auto &functions = ir->getFunctions();
	for (auto &function : functions) {
		auto &vars = function->getVars();
		for (auto &var : vars) {
			auto &&iteratedDF = computeIteratedDF(var->getDefBlocks());
			// insert phi-function at DF+
			for (auto &dfBlk : iteratedDF)
				dfBlk->append_front(std::make_shared<PhiFunction>(dfBlk, var));
		}
	}
}

void SSAConstructor::renameVariables()
{
	auto &functions = ir->getFunctions();
	for (auto &function : functions)
		renameVariables(function);
}

void SSAConstructor::renameVariables(std::shared_ptr<Function> func)
{
	auto &blocks = func->getBlockList();
	for (auto &block : blocks){
		for (auto instr = block->getFront(); instr != nullptr; instr = instr->getNextInstr()) {
			auto def_var = std::static_pointer_cast<VirtualReg>(instr->getDefReg());
			if (def_var != nullptr) {
				updateReachingDef(def_var, instr, func);
				auto new_var = def_var->newName(block);
				instr->setDefReg(new_var);
				new_var->setReachingDef(def_var->getReachingDef());
				def_var->setReachingDef(new_var);
			}
			if (instr->getTag() != IRInstruction::PHI) {
				auto use_regs = instr->getUseRegs();
				std::unordered_map < std::shared_ptr<Register>,std::shared_ptr<Register> > table;
				for (auto &var : use_regs) {
					updateReachingDef(def_var, instr, func);
					table[var] = std::static_pointer_cast<VirtualReg>(var)->getReachingDef();
				}
				instr->renameUseRegs(table);
			}
		}
		auto successors = block->getBlocksTo();
		for (auto &block_to : successors) {
			for (auto instr = block_to->getFront(); instr != nullptr; instr = instr->getNextInstr()) 
				if (instr->getTag() == IRInstruction::PHI) {
					auto var = std::static_pointer_cast<VirtualReg>(std::static_pointer_cast<PhiFunction>(instr)->getDst());
					std::static_pointer_cast<PhiFunction>(instr)->appendRelatedReg(var->getReachingDef());
				}
				else break;
		}
	}
}

void SSAConstructor::collectVariales()
{
	auto &functions = ir->getFunctions();
	for (auto &function : functions) {
		auto blocks = function->getBlockList();
		for (auto &block : blocks) {
			std::unordered_set<std::shared_ptr<Register> > local;
			auto instr = block->getFront();
			while (instr != nullptr) {
				auto useRegs = instr->getUseRegs();
				for (auto &reg : useRegs) {
					if (Operand::isRegister(reg->category()) && local.find(reg) == local.end())
						function->append_var(std::static_pointer_cast<VirtualReg>(reg));
				}
				auto def = instr->getDefReg();
				instr = instr->getNextInstr();
				if (def != nullptr && Operand::isRegister(def->category())) {
					local.insert(def);
					std::static_pointer_cast<VirtualReg>(def)->append_def_block(block);
				}
			}
		}
	}
}

void SSAConstructor::updateReachingDef
(std::shared_ptr<VirtualReg> v, std::shared_ptr<IRInstruction> i, std::shared_ptr<Function> f)
{
	auto r = v->getReachingDef();
	while (r != nullptr && f->getDT()->isDominating(r->getDefiningBlock(), i->getBlock()))
		r = r->getReachingDef();
	v->setReachingDef(r);
}

std::unordered_set<std::shared_ptr<BasicBlock>> 
SSAConstructor::computeIteratedDF(const std::vector<std::shared_ptr<BasicBlock> > &S)
{
	std::unordered_set<std::shared_ptr<BasicBlock>> ret;
	visited.clear();
	onceInQueue.clear();
	for (auto &node : S) insertNode(node);
	while (!workQueue.empty()) {
		auto x = workQueue.top(); 
		workQueue.pop();
		visited.insert(x);
		visit(x, x, ret);
	}
	return ret;
}

void SSAConstructor::visit(std::shared_ptr<BasicBlock> y,const  std::shared_ptr<BasicBlock> &current_x,
	std::unordered_set<std::shared_ptr<BasicBlock>>& DFplus)
{
	auto &JEdges = y->getDTInfo().JEdges;
	auto &DEdges = y->getDTInfo().DEdges;

	for (auto z : JEdges) 
		if (z->getDTInfo().depth < current_x->getDTInfo().depth) {
			if (DFplus.find(z) == DFplus.end()) {
				DFplus.insert(z);
				if (onceInQueue.find(z) == onceInQueue.end()) insertNode(z);
			}
		}
	for (auto z : DEdges)
		if (visited.find(z) == visited.end()) {
			visited.insert(z);
			visit(z,current_x,DFplus);
		}
}

void SSAConstructor::insertNode(std::shared_ptr<BasicBlock> x)
{
	workQueue.push(x);
	onceInQueue.insert(x);
}


