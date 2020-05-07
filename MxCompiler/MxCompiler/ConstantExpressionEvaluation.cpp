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
		if (tag == IRInstruction::QUADR) {
			auto quad = std::static_pointer_cast<Quadruple>(i);
			auto op = quad->getOp();
			if (op == Quadruple::LOAD || op == Quadruple::STORE) continue;
			else if (op == Quadruple::INV) {
				if (quad->getSrc1()->category() == Operand::IMM) {
					auto move = std::make_shared<Quadruple>(quad->getBlock(), Quadruple::MOVE, quad->getDst(),
						std::make_shared<Immediate>(~std::static_pointer_cast<Immediate>(quad->getSrc1())->getValue()));
					replaceInstruction(i,move);
					Q.push(move);
					inQ.insert(move);
				}
			}
			else if (op == Quadruple::NEG) {
				if (quad->getSrc1()->category() == Operand::IMM) {
					auto move = std::make_shared<Quadruple>(quad->getBlock(), Quadruple::MOVE, quad->getDst(),
						std::make_shared<Immediate>(-std::static_pointer_cast<Immediate>(quad->getSrc1())->getValue()));
					replaceInstruction(i, move);
					Q.push(move);
					inQ.insert(move);
				}
			}
			else if (op == Quadruple::MOVE) {
				auto tp = quad->getSrc1()->category();
				if (tp == Operand::IMM)
					propagateImm(i, std::static_pointer_cast<Register>(quad->getDst()),
						std::static_pointer_cast<Immediate>(quad->getSrc1()));
				else if (tp == Operand::REG_REF || tp == Operand::REG_VAL)
					copyPropagate(i, std::static_pointer_cast<Register>(quad->getDst()),
						std::static_pointer_cast<Register>(quad->getSrc1()));
			}
			else optimizeBinaryExpr(quad);
		}
		else if (tag == IRInstruction::CALL)
			optimizeCall(std::static_pointer_cast<Call>(i));
	}
}

void ConstantExpressionEvaluation::optimizeBinaryExpr(std::shared_ptr<Quadruple> q)
{
	if (q->getSrc1()->category() != Operand::IMM || q->getSrc2()->category() != Operand::IMM) return;
	int x = std::static_pointer_cast<Immediate>(q->getSrc1())->getValue();
	int y = std::static_pointer_cast<Immediate>(q->getSrc2())->getValue();
	int result;
	switch (q->getOp())
	{
	case Quadruple::ADD: result = x + y; break;
	case Quadruple::MINUS: result = x - y; break;
	case Quadruple::TIMES: result = x * y; break;
	case Quadruple::DIVIDE: result = x / y; break;
	case Quadruple::MOD: result = x % y; break;
	case Quadruple::LSHIFT: result = x << y; break;
	case Quadruple::RSHIFT: result = x >> y; break;
	case Quadruple::BITAND: result = x & y; break;
	case Quadruple::BITOR: result = x | y; break;
	case Quadruple::BITXOR: result = x ^ y; break;
	case Quadruple::LESS: result = x < y; break;
	case Quadruple::LEQ: result = x <= y; break;
	case Quadruple::GREATER: result = x > y; break;
	case Quadruple::GEQ: result = x >= y; break;
	case Quadruple::EQ: result = x == y; break;
	case Quadruple::NEQ: result = x != y; break;
	default:
		break;
	}
	auto move = std::make_shared<Quadruple>(q->getBlock(), Quadruple::MOVE, q->getDst(),
		std::make_shared<Immediate>(result));
	replaceInstruction(q, move);
	Q.push(move);
	inQ.insert(move);
}

void ConstantExpressionEvaluation::optimizeCall(std::shared_ptr<Call> c)
{
	auto f = c->getFunction();

	if (f == ir->toString) {
		if (c->getArgs()[0]->category() == Operand::IMM) {
			changed = true;
			int val = std::static_pointer_cast<Immediate>(c->getArgs()[0])->getValue();
			auto reg = std::make_shared<VirtualReg>(Operand::REG_VAL, "__str");
			auto str = std::make_shared<StaticString>(reg, std::to_string(val));
			ir->addStringConst(str);
			auto move = std::make_shared<Quadruple>(c->getBlock(), Quadruple::MOVE, c->getResult(), reg);
			replaceInstruction(c, move);
		}
		return;
	}
	if (!ir->isStringFunction(f)) return;
	
	bool isThisStaticStr = c->getObjRef()->category() == Operand::STATICSTR;
	bool isArg1StaticStr = c->getArgs().size() > 0 && c->getArgs()[0]->category() == Operand::STATICSTR;
	bool isArg2StaticStr = c->getArgs().size() > 1 && c->getArgs()[1]->category() == Operand::STATICSTR;
	
	auto obj = isThisStaticStr ? std::static_pointer_cast<StaticString>(c->getObjRef()) : nullptr;
	auto lhs = isArg1StaticStr ? std::static_pointer_cast<StaticString>(c->getArgs()[0]) : nullptr;
	auto rhs = isArg2StaticStr ? std::static_pointer_cast<StaticString>(c->getArgs()[1]) : nullptr;
	if (f == ir->stringAdd) {
		if (isArg1StaticStr && isArg2StaticStr) {
			changed = true;
			auto reg = std::make_shared<VirtualReg>(Operand::REG_VAL, "__str");
			auto str = std::make_shared<StaticString>(reg, lhs->getText() + rhs->getText());
			auto move = std::make_shared<Quadruple>(c->getBlock(), Quadruple::MOVE, c->getResult(), reg);
			ir->addStringConst(str);
			replaceInstruction(c, move);
		}
	}
	else if (f == ir->stringEQ) {
		if (isArg1StaticStr && isArg2StaticStr) {
			changed = true;
			auto move = std::make_shared<Quadruple>(c->getBlock(), Quadruple::MOVE, c->getResult(),
				std::make_shared<Immediate>(lhs->getText() == rhs->getText()));
			replaceInstruction(c, move);
		}
	}
	else if (f == ir->stringNEQ) {
		if (isArg1StaticStr && isArg2StaticStr) {
			changed = true;
			auto move = std::make_shared<Quadruple>(c->getBlock(), Quadruple::MOVE, c->getResult(),
				std::make_shared<Immediate>(lhs->getText() != rhs->getText()));
			replaceInstruction(c, move);

		}
	}
	else if (f == ir->stringGEQ) {
		if (isArg1StaticStr && isArg2StaticStr) {
			changed = true;
			auto move = std::make_shared<Quadruple>(c->getBlock(), Quadruple::MOVE, c->getResult(),
				std::make_shared<Immediate>(lhs->getText() >= rhs->getText()));
			replaceInstruction(c, move);
		}
	}
	else if (f == ir->stringLEQ) {
		if (isArg1StaticStr && isArg2StaticStr) {
			changed = true;
			auto move = std::make_shared<Quadruple>(c->getBlock(), Quadruple::MOVE, c->getResult(),
				std::make_shared<Immediate>(lhs->getText() <= rhs->getText()));
			replaceInstruction(c, move);
		}
	}
	else if (f == ir->stringLESS) {
		if (isArg1StaticStr && isArg2StaticStr) {
			changed = true;
			auto move = std::make_shared<Quadruple>(c->getBlock(), Quadruple::MOVE, c->getResult(),
				std::make_shared<Immediate>(lhs->getText() < rhs->getText()));
			replaceInstruction(c, move);

		}
	}
	else if (f == ir->stringGREATER) {
		if (isArg1StaticStr && isArg2StaticStr) {
			changed = true;
			auto move = std::make_shared<Quadruple>(c->getBlock(), Quadruple::MOVE, c->getResult(),
				std::make_shared<Immediate>(lhs->getText() > rhs->getText()));
			replaceInstruction(c, move);
		}
	}
	else if (f == ir->ord) {
		if (isThisStaticStr && c->getArgs()[0]->category() == Operand::IMM) {
			changed = true;
			int pos = std::static_pointer_cast<Immediate>(c->getArgs()[0])->getValue();
			replaceInstruction(c,std::make_shared<Quadruple>(
				c->getBlock(), Quadruple::MOVE, c->getResult(),
				std::make_shared<Immediate>((int)obj->getText()[pos])));
		}
	}
	else if (f == ir->substring) {
		if (isThisStaticStr && c->getArgs()[0]->category() == Operand::IMM && c->getArgs()[1]->category() == Operand::IMM){
			changed = true;
			int l = std::static_pointer_cast<Immediate>(c->getArgs()[0])->getValue();
			int r = std::static_pointer_cast<Immediate>(c->getArgs()[1])->getValue();
			auto reg = std::make_shared<VirtualReg>(Operand::REG_VAL, "__str");
			auto str = std::make_shared<StaticString>(reg, obj->getText().substr(l, r - l + 1));
			replaceInstruction(c,std::make_shared<Quadruple>(c->getBlock(), Quadruple::MOVE, c->getResult(), reg));
			ir->addStringConst(str);
		}
	}
	else if (f == ir->parseInt) {
		if (isThisStaticStr) {
			changed = true;
			int x;
			std::stringstream ss(obj->getText());
			ss >> x;
			replaceInstruction(c,std::make_shared<Quadruple>(c->getBlock(), Quadruple::MOVE, c->getResult(),
				std::make_shared<Immediate>(x)));
		}
	}
	else if (f == ir->stringLength) {
		if (isThisStaticStr) {
			changed = true;
			replaceInstruction(c,std::make_shared<Quadruple>(c->getBlock(), Quadruple::MOVE, c->getResult(),
				std::make_shared<Immediate>(obj->getText().length())));
		}
	}
}

void ConstantExpressionEvaluation::propagateImm(std::shared_ptr<IRInstruction> i, std::shared_ptr<Register> old, std::shared_ptr<Immediate> _new)
{
	bool usedByPhi = false;
	auto users = use[old];
	for(auto &user : users)
		if (user != i) {
			if (user->getTag() == IRInstruction::PHI) usedByPhi = true;  // keep phi-functions unchanged
			else {
				changed = true;
				use[old].erase(user);
				user->replaceUseReg(old, _new);
				if (inQ.find(user) == inQ.end()) {
					Q.push(user);
					inQ.insert(user);
				}
			}
		}
	if (!usedByPhi) removeInstruction(i);
}

void ConstantExpressionEvaluation::copyPropagate(std::shared_ptr<IRInstruction> i, std::shared_ptr<Register> old, std::shared_ptr<Register> _new)
{
	bool usedByPhi = false;
	auto users = use[old];
	auto &usersOfNewReg = use[_new];
	for (auto &user : users)
		if (user != i) {
			if (user->getTag() == IRInstruction::PHI) usedByPhi = true;  // keep phi-functions unchanged
			else {
				changed = true;
				use[old].erase(user);
				user->replaceUseReg(old, _new);
				usersOfNewReg.insert(user);
				if (inQ.find(user) == inQ.end()) {
					Q.push(user);
					inQ.insert(user);
				}
			}
		}
	if (!usedByPhi) {
		usersOfNewReg.erase(i);
		removeInstruction(i);
	}
}


