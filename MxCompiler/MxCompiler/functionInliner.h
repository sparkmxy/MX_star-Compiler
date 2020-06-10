#pragma once

#include "pch.h"
#include "IR.h"
#include "IRinstruction.h"
#include "basicblock.h"
#include "cfg_pass.h"

class FunctionInliner :public CFG_Pass{
public:
	FunctionInliner(std::shared_ptr<IR> _ir) :CFG_Pass(_ir) {}
	bool run() override;
private:

	static const int MAX_INLINING_DEPTH = 4, INLINE_THRESHOLD = 3;

	std::map<std::shared_ptr<Function>, int> call_cnt, instr_cnt;

	std::map<std::shared_ptr<Operand>, std::shared_ptr<Operand> > shadowOperands;
	std::map<std::shared_ptr<BasicBlock>, std::shared_ptr<BasicBlock> > shadowBlocks;
	std::map<std::shared_ptr<Function>, std::shared_ptr<Function> > shadows;



	void prepare();
	void non_recursive();
	void recursive();

	std::shared_ptr<IRInstruction> expand(std::shared_ptr<Call> c);
	
	bool okForInlining(std::shared_ptr<Function> f);

	bool okForRecursivelyInlining(std::shared_ptr<Function> f);

	// create shadow functions for recursive functions
	void createShadows();

	std::shared_ptr<Function> createShadowFunction(std::shared_ptr<Function> f);

	void setShadowOperand(std::shared_ptr<Operand> op);

	void globalVarInit();
};