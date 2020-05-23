#include "registerAllocator.h"

void RegisterAllocator::run()
{
	auto functions = program->getFunctions();
	for (auto &function : functions) {
		f = function;
		while (true)
		{
			init();
			livenessAnalysis();
			buildInferenceGraph();
			collect();
			while (!simplifySet.empty() || !moveSet.empty() || freezeSet.empty() || spillSet.empty()) {
				if (!simplifySet.empty()) simplify();
				else if (!moveSet.empty()) coalesce();
				else if (!freezeSet.empty()) freeze();
				else select();
			}
			assignColor();
			if (spilled.empty()) break;
			rewrite();
		}
	}
}

void RegisterAllocator::init()
{
	clear();
	// reset the precolored registers
	program->resetPrecoloredRegs();

	for(auto b : f->getBlockList())
		for (auto i = b->getFront(); i != nullptr; i = i->getNextInstr()) {
			for (auto reg : i->getUseReg()) initial.insert(reg);
			if (i->getDefReg() != nullptr) initial.insert(i->getDefReg());
		}
	for (auto reg : initial)  reg->info().clear();
}

void RegisterAllocator::buildInferenceGraph()
{
	for (auto b : f->getBlockList()) {
		auto live = liveout[b];
		for (auto i = b->getFront(); i = nullptr; i = i->getNextInstr()) {
			if (i->category() == RISCVinstruction::MOV) {
				for (auto reg : i->getUseReg()) live.erase(reg);
				
				auto move = std::static_pointer_cast<MoveAssembly>(i);
				i->getDefReg()->info().moveList.insert(move);
				for (auto reg : i->getUseReg()) reg->info().moveList.insert(move);	
				moveSet.insert(move);
			}

			std::vector<std::shared_ptr<Register> > defs;
			if (i->getDefReg() != nullptr) defs.push_back(i->getDefReg());
			if (i->category() == RISCVinstruction::CALL) {
				for (auto regName : RISCVConfig::callerSaveRegNames)
					defs.push_back((*program)[regName]);
			}

			for (auto def : defs) {
				for (auto reg : live) addEdge(def, reg);
				live.erase(def);
			}

			for (auto reg : i->getUseReg()) live.insert(reg);
			// update liveout
		}
	}
}

void RegisterAllocator::livenessAnalysis()
{
	// resolve Use-Def for each block
	for (auto &b : f->getBlockList()) {
		def[b].clear();
		use[b].clear();
		livein[b].clear();
		liveout[b].clear();
		for (auto i = b->getFront(); i != nullptr; i = i->getNextInstr()) {
			for (auto reg : i->getUseReg())
				if (def[b].find(reg) == def[b].end())
					use[b].insert(reg);

			if (i->getDefReg() != nullptr) def[b].insert(i->getDefReg());

			if (i->category() == RISCVinstruction::CALL) {
				for (auto regName : RISCVConfig::callerSaveRegNames)
					def[b].insert((*program)[regName]);
			}
		}
	}

	while(true){
		auto blocks = f->getBlockList();
		for (int i = blocks.size() - 1; i >= 0; i--) {
			auto &b = blocks[i];

			std::unordered_set<std::shared_ptr<Register> > new_out;
			auto new_in = liveout[b];
			for (auto reg : def[b]) new_in.erase(reg);
			for (auto reg : use[b]) new_in.insert(reg);
			for (auto bb : b->getToBlocks())
				for (auto reg : livein[bb]) new_out.insert(reg);
			
			if (new_in == livein[b] && new_out == liveout[b]) break;  // nothing changes
			livein[b] = new_in;
			liveout[b] = new_out;
		}
	} 
}

void RegisterAllocator::collect()
{
	for (auto reg : initial) {
		if (reg->info().degree >= K) spillSet.insert(reg);
		else if (isMoveRelated(reg)) freezeSet.insert(reg);
		else simplifySet.insert(reg);
	}
	initial.clear();
}

void RegisterAllocator::simplify()
{
	auto reg = *simplifySet.begin(); //  this cannot be a physical register
	simplifySet.erase(reg);
	selectStack.push_back(reg);
	for (auto r : getNeighbors(reg)) decreaseDegreeBy1(r);

}

void RegisterAllocator::coalesce()
{
	auto move = *moveSet.begin();
	moveSet.erase(move);
	auto x = getAlias(move->getRs1()), y = getAlias(move->getRd());
	if (preColored.find(y) != preColored.end()) std::swap(x, y);
	
	if (x == y) {
		coalescedMoves.insert(move);
		enqueue(x);
	}
	else if (preColored.find(y) != preColored.end() || edges.find(Edge(x, y)) != edges.end() || x->getName() == "zero") {
		constrainedMoves.insert(move);
		enqueue(x);
		enqueue(y);
	}
	else {
		bool cond = true;
		if (preColored.find(x) != preColored.end())
			for (auto r : getNeighbors(y)) cond &= check(r, x);
		else {
			auto temp = x->info().adjList;
			for (auto r : y->info().adjList) temp.insert(r);
			cond = isConservative(temp);
		}

		if (cond) {
			coalescedMoves.insert(move);
			union_nodes(x, y);
			enqueue(x);
		}
		else activeMoves.insert(move);
	}
}

void RegisterAllocator::freeze()
{
	auto r = *freezeSet.begin();   // this cannot be a physical register.
	freezeSet.erase(r);
	simplifySet.insert(r);
	freezeMoves(r);
}

void RegisterAllocator::select()
{
	auto reg = std::static_pointer_cast<VirtualReg>(*spillSet.begin());
	for (auto r : spillSet)
		if (isForSpill[reg] ||
			(!isForSpill[r] && spillPriority[r] / r->info().degree < spillPriority[reg] / reg->info().degree))
			reg = r;
	spillSet.erase(reg);
	simplifySet.insert(reg);
	freezeMoves(reg);
}

void RegisterAllocator::assignColor()
{
	while (!selectStack.empty()) {
		auto reg = selectStack.back();
		selectStack.pop_back();

		auto availableColors = program->getAllocatableRegs();

		for (auto r : reg->info().adjList) {
			auto r_alias = getAlias(r.lock());
			if (colored.find(r_alias) != colored.end() || preColored.find(r_alias) != preColored.end())
				availableColors.erase(r_alias->info().color.lock());
		}

		if (availableColors.empty()) spilled.insert(reg);
		else {
			colored.insert(reg);
			
			std::shared_ptr<PhysicalRegister> color;
			for(auto regName : RISCVConfig::calleeSaveRegNames)
				if (availableColors.find((*program)[regName]) != availableColors.end()) {
					color = (*program)[regName];
					break;
				}
			if (color == nullptr)     // try to find callersave registers
				for (auto regName : RISCVConfig::callerSaveRegNames)
					if (availableColors.find((*program)[regName]) != availableColors.end()) {
						color = (*program)[regName];
						break;
					}

			reg->info().color = color;
		}
	}

	for (auto r : coalesced) r->info().color = getAlias(r)->info().color;
}

void RegisterAllocator::rewrite()
{
	for (auto reg : spilled)
		reg->info().spilladdr = std::make_shared<StackLocation>(
		(*program)["sp"], f->stackLocationFromBottom(Configuration::SIZE_OF_INT), false);

	for (auto b : f->getBlockList()) {

		for (auto i = b->getFront(); i != nullptr; i = i->getNextInstr()) {

			for(auto reg : i->getUseReg())
				if (reg->info().spilladdr != nullptr) {
					auto temp = std::make_shared<VirtualReg>(Operand::REG_VAL,"spillUse");
					isForSpill[temp] = true;
					auto load = std::make_shared<Load>(reg->info().spilladdr, temp, Configuration::SIZE_OF_INT);
					appendBefore(i, load);
					i->updateUseReg(reg, temp);
				}

			if (i->getDefReg() != nullptr && i->getDefReg()->info().spilladdr != nullptr) {
				auto temp = std::make_shared<VirtualReg>(Operand::REG_VAL, "spillDef");
				isForSpill[temp] = true;

				auto reg = i->getDefReg();
				auto store = std::make_shared<Store>(reg->info().spilladdr, temp, Configuration::SIZE_OF_INT);
				appendBefore(i, store);
				i->updateDefReg(temp);
			}
		}
	}
}

void RegisterAllocator::clear()
{
	initial.clear();
	coalesced.clear();
	colored.clear();
	
	coalescedMoves.clear();
	constrainedMoves.clear();
	frozenMoves.clear();
	activeMoves.clear();
}

void RegisterAllocator::addEdge(std::shared_ptr<Register> x, std::shared_ptr<Register> y)
{
	if (x == y) return;
	auto e1 = Edge(x, y);
	if (edges.find(e1) != edges.end()) return;
	edges.insert(e1);
	edges.insert(Edge(y, x));
	if (preColored.find(y) == preColored.end()) {
		y->info().adjList.insert(x);
		y->info().degree++;
	}
	if (preColored.find(x) == preColored.end()) {
		x->info().adjList.insert(y);
		x->info().degree++;
	}
}

bool RegisterAllocator::isMoveRelated(std::shared_ptr<Register> reg)
{
	return !nodeMoves(reg).empty();
}

std::unordered_set<std::shared_ptr<MoveAssembly> > RegisterAllocator::nodeMoves(std::shared_ptr<Register> reg)
{
	std::unordered_set<std::shared_ptr<MoveAssembly> > ret;
	for (auto r : reg->info().moveList) ret.insert(r.lock());
	for (auto move : moveSet) ret.insert(move);
	for (auto move : ret)
		if (activeMoves.find(move) == activeMoves.end()) ret.erase(move);
	return ret;
}

std::unordered_set<std::shared_ptr<Register>> RegisterAllocator::getNeighbors(std::shared_ptr<Register> reg)
{
	std::unordered_set<std::shared_ptr<Register>> ret;
	for (auto r : reg->info().adjList)
		if (coalesced.find(r.lock()) == coalesced.end()) ret.insert(r.lock());
	for (auto r : selectStack) ret.erase(r);
	return ret;
}

void RegisterAllocator::decreaseDegreeBy1(std::shared_ptr<Register> reg)
{
	reg->info().degree--;
	if (reg->info().degree == K){
		auto regs = getNeighbors(reg);
		regs.insert(reg);
		enableMove(regs);

		spillSet.erase(reg);
		if (isMoveRelated(reg)) freezeSet.insert(reg);
		else simplifySet.insert(reg);
	}
}

void RegisterAllocator::enableMove(std::unordered_set<std::shared_ptr<Register>> regs)
{
	for(auto reg : regs)
		for(auto move : nodeMoves(reg))
			if (activeMoves.find(move) != activeMoves.end()) { // it is active
				activeMoves.erase(move);
				moveSet.insert(move);
			}
}

std::shared_ptr<Register> RegisterAllocator::getAlias(std::shared_ptr<Register> reg)
{
	auto r = reg;
	while (coalesced.find(r) != coalesced.end()) r = r->info().alias.lock();
	return r;
}

void RegisterAllocator::enqueue(std::shared_ptr<Register> reg)
{
	if (preColored.find(reg) == preColored.end() && !isMoveRelated(reg) && reg->info().degree < K) {
		freezeSet.erase(reg);
		simplifySet.insert(reg);
	}
}

bool RegisterAllocator::check(std::shared_ptr<Register> t, std::shared_ptr<Register> r)
{
	return t->info().degree < K || preColored.find(t) != preColored.end() || edges.find(Edge(t, r)) != edges.end();
}

bool RegisterAllocator::isConservative(std::unordered_set<std::weak_ptr<Register>> regs)
{
	int cnt = 0;
	for (auto reg : regs)
		if (reg.lock()->info().degree >= K) cnt++;

	return cnt < K;
}

void RegisterAllocator::union_nodes(std::shared_ptr<Register> x, std::shared_ptr<Register> y)
{
	freezeSet.erase(y);
	spillSet.erase(y);
	coalesced.insert(y);
	y->info().alias = x;
	for (auto move : y->info().moveList) x->info().moveList.insert(move);
	enableMove({ y });

	for (auto r : getNeighbors(y)) {
		addEdge(r, x);
		decreaseDegreeBy1(r);
	}

	if (x->info().degree >= K && freezeSet.find(x) != freezeSet.end()) {
		freezeSet.erase(x);
		spillSet.insert(x);
	}
}

void RegisterAllocator::freezeMoves(std::shared_ptr<Register> r)
{
	std::shared_ptr<Register> y;
	for (auto move : nodeMoves(r)) {
		if (getAlias(move->getRs1()) == getAlias(r)) y = getAlias(getAlias(move->getRd()));
		else y = getAlias(move->getRs1());

		activeMoves.erase(move);
		frozenMoves.insert(move);
		if (freezeSet.find(y) != freezeSet.end() && nodeMoves(y).empty()) {
			freezeSet.erase(y);
			simplifySet.insert(y);
		}
	}
}
 