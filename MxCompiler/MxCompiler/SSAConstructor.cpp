#include "SSAConstructor.h"

void SSAConstructor::constructSSA()
{
	buildDominanceTree();
	collectVariales();
	insertPhiFunction();
	renameVariables();
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
		for (auto instr = block->getFront(); instr != nullptr; instr = instr->getNextInstr())
			if (instr->getTag() == IRInstruction::PHI) {

			}
			else {
					
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
			if (DFplus.find(z) == DFplus.end) {
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


