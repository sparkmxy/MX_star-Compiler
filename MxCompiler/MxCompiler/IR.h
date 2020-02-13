#pragma once

#include "pch.h"
#include "Operand.h"
#include "Function.h"

class IR {
public:

	void addGlobalVar(std::shared_ptr<Int64Global> var);
	void addFunction(std::shared_ptr<Function> func);

private:
	std::vector<std::shared_ptr<Int64Global> > glbVars;
	std::vector<std::shared_ptr<Function> > functions;
};