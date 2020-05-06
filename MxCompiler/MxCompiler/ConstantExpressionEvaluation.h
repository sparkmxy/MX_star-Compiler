#pragma once

#include "cfg_pass.h"
#include "IRinstruction.h"
#include "IR.h"
#include<queue>

/*
Calculate constant expressions and copy propagation.
*/
class ConstantExpressionEvaluation :public CFG_Pass{
public:
	ConstantExpressionEvaluation(std::shared_ptr<IR> _ir):CFG_Pass(_ir) {}
	
	bool run() override;

private:
	bool changed;

	std::queue<std::shared_ptr<IRInstruction> > Q;
	std::unordered_set<std::shared_ptr<IRInstruction> > inQ;

	void constExprEval(std::shared_ptr<Function> f);
	void optimizeBinaryExpr(std::shared_ptr<Quadruple> q);
	void optimizeCall(std::shared_ptr<Call> c);

	void propagateImm(std::shared_ptr<IRInstruction> i, 
		std::shared_ptr<Register> old, std::shared_ptr<Immediate> _new);

	void copyPropagate(std::shared_ptr<IRInstruction> i,
		std::shared_ptr<Register> old, std::shared_ptr<Register> _new);
};

