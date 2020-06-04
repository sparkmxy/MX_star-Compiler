#include "registerAllocator.h"

RegisterAllocator::~RegisterAllocator()
{
	for(auto f : program->getFunctions())
		for(auto b : f->getBlockList())
			for (auto i = b->getFront(); i != nullptr; i = i->getNextInstr()) {
				for (auto r : i->getUseReg()) r->info().clear();
				if (i->getDefReg() != nullptr) i->getDefReg()->info().clear();
			}
}

void RegisterAllocator::run()
{
	auto functions = program->getFunctions();
	for (auto &function : functions) {
		f = function;
		f->computePreOrderList();
		while (true)
		{
			init();
			livenessAnalysis();
			buildInferenceGraph();
			collect();
			while (!simplifySet.empty() || !moveSet.empty() || !freezeSet.empty() || !spillSet.empty()) {
				if (!simplifySet.empty()) simplify();
				else if (!moveSet.empty()) coalesce();
				else if (!freezeSet.empty()) freeze();
				else select();
			}
			assignColor();
			if (spilled.empty()) break;
			rewrite();
		}
		apply();
	}
}

void RegisterAllocator::init()
{
	clear();
	for(auto b : f->getBlockList())
		for (auto i = b->getFront(); i != nullptr; i = i->getNextInstr()) {
			for (auto reg : i->getUseReg()) initial.insert(reg);
			if (i->getDefReg() != nullptr) initial.insert(i->getDefReg());
		}
	for (auto reg : initial)  reg->info().clear();
	
	// reset the precolored registers
	program->resetPrecoloredRegs();

	for (auto &b : f->getBlockList()) {
		int w = (1 << std::min(b->getToBlocks().size(), b->getFromBlocks().size()));
		for (auto i = b->getFront(); i != nullptr; i = i->getNextInstr()) {
			for (auto reg : i->getUseReg()) spillPriority[reg] += w;
			if (i->getDefReg() != nullptr) spillPriority[i->getDefReg()] += w;
		}
	}
}

void RegisterAllocator::buildInferenceGraph()
{
	for (auto b : f->getBlockList()) {
		auto live = liveout[b];
		for (auto i = b->getBack(); i != nullptr; i = i->getPrevInstr()) { // reversely
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

			if (i->category() == RISCVinstruction::STORE) {
				auto s = std::static_pointer_cast<Store>(i);
				if (s->getRt() != nullptr) addEdge(s->getRs(), s->getRt());
			}
			for (auto def : defs) 
				for (auto reg : live) addEdge(def, reg);
			
			for (auto reg : defs) live.erase(reg);
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
				if (def[b].find(reg) == def[b].end()) use[b].insert(reg);

			if (i->getDefReg() != nullptr) def[b].insert(i->getDefReg());

			if (i->category() == RISCVinstruction::CALL) {
				for (auto regName : RISCVConfig::callerSaveRegNames)
					def[b].insert((*program)[regName]);
			}
		}
	}

	bool changed;
	do{
		changed = false;
		auto blocks = f->getBlockList();
		for (int i = blocks.size() - 1; i >= 0; i--) {
			auto &b = blocks[i];

			std::set<std::shared_ptr<Register> > new_out, new_in = liveout[b];
			for (auto reg : def[b]) new_in.erase(reg);
			for (auto reg : use[b]) new_in.insert(reg);
			for (auto bb : b->getToBlocks())
				for (auto reg : livein[bb]) new_out.insert(reg);
			
			if (new_in != livein[b] || new_out != liveout[b]) {
				changed = true;  
				livein[b] = new_in;
				liveout[b] = new_out;
			}
		}
	} while (changed);
}

void RegisterAllocator::collect()
{
	for (auto reg : initial) 
		if(reg->category() != Operand::PHISICAL){
			if (reg->info().degree >= K) 
				spillSet.insert(reg);
			else if (isMoveRelated(reg)) freezeSet.insert(reg);
			else simplifySet.insert(reg);
		}
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
			auto temp = getNeighbors(x);
			for (auto r : getNeighbors(y)) temp.insert(r);
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
	auto reg = *spillSet.begin();

	for (auto r : spillSet)
		if (isForSpill[reg] || (!isForSpill[r] &&
			(double)spillPriority[r] / r->info().degree < (double)spillPriority[reg] / reg->info().degree))
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
			auto r_alias = getAlias(r);
			if (colored.find(r_alias) != colored.end() || preColored.find(r_alias) != preColored.end())
				availableColors.erase(r_alias->info().color);
		}

		if (availableColors.empty()) 
				spilled.insert(reg);
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

			if (color == nullptr)
				throw Error("unblievable");
			reg->info().color = color;
		}
	}

	for (auto r : coalesced) r->info().color = getAlias(r)->info().color;
}

void RegisterAllocator::rewrite()
{
	for (auto reg : spilled)
		reg->info().spilladdr = std::make_shared<StackLocation>(f,
		(*program)["sp"], f->stackLocationFromBottom(Configuration::SIZE_OF_INT));

	for (auto b : f->getBlockList()) {

		for (auto i = b->getFront(); i != nullptr; i = i->getNextInstr()) {

			auto useRegs = i->getUseReg();
			if (useRegs.size() == 2 && useRegs[1] == useRegs[0]) useRegs.pop_back();
			
			for(auto reg : useRegs)
				if (reg->info().spilladdr != nullptr) {
					auto temp = std::make_shared<VirtualReg>(Operand::REG_VAL,"spillUse");
					isForSpill[temp] = true;
					auto load = std::make_shared<Load>(i->getBlock(), reg->info().spilladdr, temp, Configuration::SIZE_OF_INT);
					appendBefore(i, load);
					i->updateUseReg(reg, temp);

					if (i->getDefReg() == reg) {
						appendAfter(i, std::make_shared<Store>(i->getBlock(), 
							reg->info().spilladdr, temp, Configuration::SIZE_OF_INT));
						i->updateDefReg(temp);
					}
				}

			auto def = i->getDefReg();
			bool used = false;
			for (auto reg : i->getUseReg()) if (reg == def) used = true;

			if (def != nullptr && def->info().spilladdr != nullptr && !used) {
				auto temp = std::make_shared<VirtualReg>(Operand::REG_VAL, "spillDef");
				isForSpill[temp] = true;

				auto store = std::make_shared<Store>(i->getBlock(), def->info().spilladdr, temp, Configuration::SIZE_OF_INT);
				appendAfter(i, store);
				i->updateDefReg(temp);
			}
		}
	}
}

void RegisterAllocator::apply()   // apply the current coloring
{
	for(auto b : f->getBlockList())
		for (auto i = b->getFront(); i != nullptr; i = i->getNextInstr()) {
			for(auto reg : i->getUseReg())
				if (reg->category() != Operand::PHISICAL) {
					if(reg->info().color != nullptr)
						i->updateUseReg(reg, reg->info().color);
					else throw Error("what fucking coloring are you doing?");
				}
			auto def = i->getDefReg();
			if (def != nullptr && def->category() != Operand::PHISICAL && def->info().color != nullptr)
				i->updateDefReg(def->info().color);
		}
}

void RegisterAllocator::clear()
{
	initial.clear();
	spilled.clear();
	coalesced.clear();
	colored.clear();
	selectStack.clear();
	
	coalescedMoves.clear();
	constrainedMoves.clear();
	frozenMoves.clear();
	activeMoves.clear();

	spillPriority.clear();
	
	edges.clear();
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

std::set<std::shared_ptr<MoveAssembly> > RegisterAllocator::nodeMoves(std::shared_ptr<Register> reg)
{
	std::set<std::shared_ptr<MoveAssembly> > ret;
	for (auto move : reg->info().moveList)
		if (activeMoves.find(move) != activeMoves.end() || moveSet.find(move) != moveSet.end()) ret.insert(move);
	return ret;
}

std::set<std::shared_ptr<Register>> RegisterAllocator::getNeighbors(std::shared_ptr<Register> reg)
{
	std::set<std::shared_ptr<Register>> ret;
	for (auto r : reg->info().adjList)
		if (coalesced.find(r) == coalesced.end()) ret.insert(r);
	for (auto r : selectStack) ret.erase(r);
	return ret;
}

void RegisterAllocator::decreaseDegreeBy1(std::shared_ptr<Register> reg)
{
	reg->info().degree--;
	if (reg->info().degree == K - 1){
		auto regs = getNeighbors(reg);
		regs.insert(reg);
		enableMove(regs);

		spillSet.erase(reg);
		if (isMoveRelated(reg)) freezeSet.insert(reg);
		else simplifySet.insert(reg);
	}
}

void RegisterAllocator::enableMove(std::set<std::shared_ptr<Register> > regs)
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
	while (coalesced.find(r) != coalesced.end()) r = r->info().alias;
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

bool RegisterAllocator::isConservative(std::set<std::shared_ptr<Register>> regs)
{
	int cnt = 0;
	for (auto reg : regs)
		if (reg->info().degree >= K) cnt++;

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
 