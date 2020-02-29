#pragma once

#include "pch.h"
#include "Operand.h"
#include "Function.h"

class IR {
public:
	IR();
	void addGlobalVar(std::shared_ptr<Register> var);
	void addFunction(std::shared_ptr<Function> func);
	
	std::shared_ptr<Function> stringLength, substring, parseInt, ord, print, println;
	std::shared_ptr<Function> stringAdd, stringNEQ, stringEQ;
	std::shared_ptr<Function> getInt, getString;
	std::shared_ptr<Function> toString, printInt, printlnInt;

	std::vector<std::shared_ptr<Function> > & getFunctions() { return functions; }
	std::vector<std::shared_ptr<Register> > & getGlbVars() { return glbVars; }
private:
	std::vector<std::shared_ptr<Register> > glbVars;
	std::vector<std::shared_ptr<Function> > functions;
	
	std::shared_ptr<Function> newBuiltInFunc(const std::string &name);
};