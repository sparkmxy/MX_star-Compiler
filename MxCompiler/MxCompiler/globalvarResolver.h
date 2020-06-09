#pragma once

#include "IR.h"
#include "IRinstruction.h"
#include "Operand.h"
#include "basicblock.h"
#include "Function.h"
#include <set>

class GlobalVarResolver {
public:
	GlobalVarResolver(std::shared_ptr<IR> _ir) :ir(_ir) {}

	void run();

private:
	std::shared_ptr<IR> ir;

	std::map<std::shared_ptr<Function>, std::set<std::shared_ptr<Function> > >
		calleeSet, recursiveCalleeSet;


	std::map<std::shared_ptr<Function>, std::set<std::shared_ptr<VirtualReg> > > 
		varDef, varUsedRecursively, varDefRecursively;

	std::map<std::shared_ptr<Function>,
		std::map<std::shared_ptr<VirtualReg>, std::shared_ptr<VirtualReg> > > func2varMap;

	void prepare();
	void loadWhenEntering();
	void storeWhenExiting();
	void resolveCallInstr();

	void computeRecursiveCalleeSet();
	// helper functions
	std::shared_ptr<VirtualReg> getTempReg(std::shared_ptr<VirtualReg> reg, std::shared_ptr<Function> f);
};
