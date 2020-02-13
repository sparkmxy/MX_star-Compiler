#include "IR.h"

void IR::addGlobalVar(std::shared_ptr<Int64Global> var)
{
	glbVars.emplace_back(var);
}

void IR::addFunction(std::shared_ptr<Function> func)
{
	functions.emplace_back(func);
}
