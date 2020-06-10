#include "Function.h"


Function::~Function()
{
	for (auto &b : blocks) b->destroyEdges();
}

void Function::appendReturnInstr(std::shared_ptr<Return> ret)
{
	retInstrs.emplace_back(ret);
}

void Function::collectReturns()
{
	retInstrs.clear();
	for (auto &b : blocks)
		for (auto i = b->getFront(); i != nullptr; i = i->getNextInstr())
			if (i->getTag() == IRInstruction::RET)
				retInstrs.push_back(std::static_pointer_cast<Return>(i));
}

void Function::appendArg(std::shared_ptr<Register> arg)
{
	args.emplace_back(arg);
}

