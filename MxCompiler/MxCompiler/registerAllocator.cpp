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
 