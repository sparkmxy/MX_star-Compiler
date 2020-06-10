#pragma once

#include "pch.h"
#include "IR.h"
#include "Function.h"
#include "basicblock.h"
#include "dominance.h"

/*
Class : CFG
Note : This class recieves an IR and provides several algorithms that can be 
conducted on the CFG.
*/
class CFG_Pass{
public:
	CFG_Pass(std::shared_ptr<IR> _ir) :ir(_ir) {}
	
	virtual bool run() = 0;
protected:
	std::shared_ptr<IR> ir;

	// def-use chain
	std::map<std::shared_ptr<Register>, std::shared_ptr<IRInstruction> > def;
	std::map<std::shared_ptr<Register>, 
		std::set<std::shared_ptr<IRInstruction> > > use;
	
	// callee set
	std::map<std::shared_ptr<Function>, std::set<std::shared_ptr<Function> > >
		calleeSet, recursiveCalleeSet;


	void resolveDefineUseChain(std::shared_ptr<Function> f);

	void computeRecursiveCalleeSet();

	void updateDTinfo(std::shared_ptr<Function> f = nullptr);
private:
};

