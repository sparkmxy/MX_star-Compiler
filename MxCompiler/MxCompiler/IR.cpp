#include "IR.h"

IR::IR()
{
	auto intType = std::make_shared<BuiltinType>(BuiltinType::INT);
	auto stringType = std::make_shared<BuiltinType>(BuiltinType::STRING);
	auto boolType = std::make_shared<BuiltinType>(BuiltinType::BOOL);

	stringLength = newBuiltInFunc("string.length", intType);
	substring = newBuiltInFunc("string.substring", stringType);
	parseInt = newBuiltInFunc("string.parseInt", intType);
	ord = newBuiltInFunc("string.ord", stringType);
	print = newBuiltInFunc("print");
	println = newBuiltInFunc("println");

	stringAdd = newBuiltInFunc("string.add", stringType);
	stringNEQ = newBuiltInFunc("string.neq", boolType);
	stringEQ = newBuiltInFunc("string.eq", boolType);

	getInt = newBuiltInFunc("getInt", intType);
	getString = newBuiltInFunc("getString", stringType);

	toString = newBuiltInFunc("toString", stringType);
	printInt = newBuiltInFunc("printInt");
	printlnInt = newBuiltInFunc("printlnInt");
}

void IR::addGlobalVar(std::shared_ptr<Register> var)
{
	glbVars.push_back(var);
}

void IR::addFunction(std::shared_ptr<Function> func)
{
	functions.push_back(func);
}

std::shared_ptr<Function> IR::newBuiltInFunc(const std::string & name, std::shared_ptr<Type> retType)
{
	return newFunction(name, retType);
}

std::shared_ptr<Function> newFunction(const std::string & _name, std::shared_ptr<Type> retType)
{
	auto f = std::make_shared<Function>(_name, retType);
	auto _entry = std::make_shared<BasicBlock>(f, BasicBlock::ENTRY);
	auto _exit = std::make_shared<BasicBlock>(f, BasicBlock::EXIT);
	f->setEntry(_entry);
	f->setExit(_exit);
	return f;
}
