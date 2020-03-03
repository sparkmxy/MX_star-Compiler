#pragma once

#include "pch.h"
#include "IR.h"
#include "Function.h"
#include "basicblock.h"

/*
Class : CFG
Note : This class recieves an IR and provides several algorithms that can be 
conducted on the CFG.
*/
class CFG_Pass{
public:
	CFG_Pass(std::shared_ptr<IR> _ir) :ir(_ir) {}
	
protected:
	std::shared_ptr<IR> ir;

	// def-use chain
	std::unordered_map<std::shared_ptr<Register>, std::shared_ptr<IRInstruction> > def;
	std::unordered_map<std::shared_ptr<Register>, 
		std::unordered_set<std::shared_ptr<IRInstruction> > > use;
private:
};

