#include "IR.h"

IR::IR()
{
	stringLength = newBuiltInFunc("string.length");
	substring = newBuiltInFunc("string.substring");
	parseInt = newBuiltInFunc("string.parseInt");
	ord = newBuiltInFunc("string.ord");
	print = newBuiltInFunc("print");
	println = newBuiltInFunc("println");

	stringAdd = newBuiltInFunc("string.add");
	stringNEQ = newBuiltInFunc("string.neq");
	stringEQ = newBuiltInFunc("string.eq");

	getInt = newBuiltInFunc("getInt");
	getString = newBuiltInFunc("getString");

	toString = newBuiltInFunc("toString");
	printInt = newBuiltInFunc("printInt");
	printlnInt = newBuiltInFunc("printlnInt");
}

void IR::addGlobalVar(std::shared_ptr<Register> var)
{
	glbVars.emplace_back(var);
}

void IR::addFunction(std::shared_ptr<Function> func)
{
	functions.emplace_back(func);
}

std::shared_ptr<Function> IR::newBuiltInFunc(const std::string & name)
{
	return std::make_shared<Function>(name);
}
