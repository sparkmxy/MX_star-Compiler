#include "ConstantExpressionEvaluation.h"

bool ConstantExpressionEvaluation::run()
{
	auto &functions = ir->getFunctions();
	for (auto &f : functions) {
		resolveDefineUseChain(f);
		constExprEval(f);
	}
	return changed;
}


// Q and inQ is always empty in the begining
void ConstantExpressionEvaluation::constExprEval(std::shared_ptr<Function> f)
{
	auto &blocks = f->getBlockList();
	for (auto &b : blocks)
		for (auto i = b->getFront(); i != nullptr; i = i->getNextInstr()) {
			Q.push(i);
			inQ.insert(i);
		}

	while (!Q.empty()) {
		auto i = Q.front();
		Q.pop();
		inQ.erase(i);
		auto tag = i->getTag();
		if (tag == IRInstruction::PHI) {

		}
		else if (tag == IRInstruction::QUADR) {
			auto quad = std::static_pointer_cast<Quadruple>(i);
			auto op = quad->getOp();
			if (op == Quadruple::LOAD || op == Quadruple::STORE) continue;
			else if (op == Quadruple::INV) {

			}
			else if (op == Quadruple::NEG) {

			}
			else if (op == Quadruple::MOVE) {

			}
			else optimizeBinaryExpr(quad);
		}
		else if (tag == IRInstruction::CALL)
			optimizeCall(std::static_pointer_cast<Call>(i));
	}
}

void ConstantExpressionEvaluation::optimizeBinaryExpr(std::shared_ptr<Quadruple> q)
{
}

void ConstantExpressionEvaluation::optimizeCall(std::shared_ptr<Call> c)
{
}

void ConstantExpressionEvaluation::propagateImm(std::shared_ptr<IRInstruction> i, std::shared_ptr<Operand> old, std::shared_ptr<Immediate> _new)
{
}

void ConstantExpressionEvaluation::copyPropagate(std::shared_ptr<IRInstruction> i, std::shared_ptr<Operand> old, std::shared_ptr<Register> _new)
{
}


