#include "Function.h"

void Function::appendReturnInstr(std::shared_ptr<Return> ret)
{
	retInstrs.emplace_back(ret);
}

void Function::appendArg(std::shared_ptr<Register> arg)
{
	args.emplace_back(arg);
}

