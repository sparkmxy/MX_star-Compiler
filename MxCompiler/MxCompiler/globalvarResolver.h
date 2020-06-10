#pragma once

#include "IR.h"
#include "IRinstruction.h"
#include "Operand.h"
#include "basicblock.h"
#include "Function.h"
#include "cfg_pass.h"
#include <set>

class GlobalVarResolver :public CFG_Pass{
public:
	GlobalVarResolver(std::shared_ptr<IR> _ir) :CFG_Pass(_ir) {}

	bool run() override;

private:


	std::map<std::shared_ptr<Function>, std::set<std::shared_ptr<VirtualReg> > > 
		varDef, varUsedRecursively, varDefRecursively;

	std::map<std::shared_ptr<Function>,
		std::map<std::shared_ptr<VirtualReg>, std::shared_ptr<VirtualReg> > > func2varMap;

	void prepare();
	void loadWhenEntering();
	void storeWhenExiting();
	void resolveCallInstr();


	// helper functions
	std::shared_ptr<VirtualReg> getTempReg(std::shared_ptr<VirtualReg> reg, std::shared_ptr<Function> f);
};
