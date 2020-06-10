#include "SSAConstructor.h"

bool SSAConstructor::run()
{
	for(auto &f : ir->getFunctions())
		f->setDT(std::make_shared<DominatorTree>(f));
	collectVariales();
	insertPhiFunction();
	renameVariables();

	ir->setSSAflag(true);
	return true;
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
	for (auto &function : functions) {
		// define parameters
		auto &args = function->getArgs();
		for(int i = 0;i<args.size();i++) {
			auto def_var = std::static_pointer_cast<VirtualReg>(args[i]);
			auto new_var = def_var->newName(nullptr); // is this ok?
			def_var->setReachingDef(new_var);
			args[i] = new_var;
		}

		if (function->getObjRef() != nullptr) {
			auto def_var = std::static_pointer_cast<VirtualReg>(function->getObjRef());
			auto new_var = def_var->newName(nullptr); // is this ok?
			def_var->setReachingDef(new_var);
			function->setObjRef(new_var);
		}
		renameVariables(function);
	}
}

void SSAConstructor::renameVariables(std::shared_ptr<Function> func)
{
	auto &blocks = func->getBlockList();
	for (auto &block : blocks){
		for (auto instr = block->getFront(); instr != nullptr; instr = instr->getNextInstr()) {
			// first update used registers, then define new ones.

			if (instr->getTag() != IRInstruction::PHI) {
				auto use_regs = instr->getUseRegs();
				std::map < std::shared_ptr<Register>, std::shared_ptr<Register> > table;
				for (auto &var : use_regs) 
					if(!var->isGlobal()){
						updateReachingDef(std::static_pointer_cast<VirtualReg>(var), instr, func);
						auto new_var = std::static_pointer_cast<VirtualReg>(var)->getReachingDef();
						if (new_var == nullptr)
							throw Error("SSA: reaching def is nullptr");
						table[var] = new_var;
						// What if reachingDef is nullptr?
					}
				instr->renameUseRegs(table);
			}

			if (instr->getDefReg() != nullptr && !instr->getDefReg()->isGlobal()) {
				auto def_var = std::static_pointer_cast<VirtualReg>(instr->getDefReg());
				updateReachingDef(def_var, instr, func);
				auto new_var = def_var->newName(block);
				instr->setDefReg(new_var);
				new_var->setReachingDef(def_var->getReachingDef());
				def_var->setReachingDef(new_var);
			}
		}
		auto successors = block->getBlocksTo();
		for (auto &block_to : successors) {
			for (auto instr = block_to->getFront(); instr != nullptr; instr = instr->getNextInstr()) 
				if (instr->getTag() == IRInstruction::PHI) {
					auto var = std::static_pointer_cast<VirtualReg>(std::static_pointer_cast<PhiFunction>(instr)->getOrigin());
					updateReachingDef(var, block->getBack(),func);
					std::static_pointer_cast<PhiFunction>(instr)->appendRelatedReg(var->getReachingDef(), block);
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
			std::set<std::shared_ptr<Register> > local;  // the variables defined in the current block
			auto instr = block->getFront();
			while (instr != nullptr) {
				auto useRegs = instr->getUseRegs();
				for (auto &reg : useRegs) {
					if (Operand::isRegister(reg->category()) && local.find(reg) == local.end())
						function->append_var(std::static_pointer_cast<VirtualReg>(reg));
				}
				auto def = instr->getDefReg();
				if (def != nullptr && Operand::isRegister(def->category())) {
					local.insert(def);
					std::static_pointer_cast<VirtualReg>(def)->append_def_block(block);
				}
				instr = instr->getNextInstr();
			}
		}
	}
}

void SSAConstructor::updateReachingDef
(std::shared_ptr<VirtualReg> v, std::shared_ptr<IRInstruction> i, std::shared_ptr<Function> f)
{
	auto r = v->getReachingDef();
	while (r != nullptr && r->getDefiningBlock() != nullptr					// args
		&&!f->getDT()->isDominating(r->getDefiningBlock(), i->getBlock()))
		r = r->getReachingDef();
	v->setReachingDef(r);
}

std::set<std::shared_ptr<BasicBlock>> 
SSAConstructor::computeIteratedDF(const std::vector<std::shared_ptr<BasicBlock> > &S)
{
	std::set<std::shared_ptr<BasicBlock>> ret;
	visited.clear();
	onceInQueue.clear();
	for (auto &node : S) insertNode(node);
	while (!workQueue.empty()) {
		auto x = workQueue.top(); 
		workQueue.pop();
		onceInQueue.erase(x);
		visited.insert(x);
		visit(x, x, ret);
	}
	return ret;
}

void SSAConstructor::visit(std::shared_ptr<BasicBlock> y,const  std::shared_ptr<BasicBlock> &current_x,
	std::set<std::shared_ptr<BasicBlock>>& DFplus)
{
	auto &JEdges = y->getDTInfo().JEdges;
	auto &DEdges = y->getDTInfo().DEdges;

	for (auto &z : JEdges) 
		if (z->getDTInfo().depth <= current_x->getDTInfo().depth) {
			if (DFplus.find(z) == DFplus.end()) {
				DFplus.insert(z);
				if (onceInQueue.find(z) == onceInQueue.end()) insertNode(z);
			}
		}
	for (auto &z : DEdges)
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


