#pragma once

#include "pch.h"
#include "Operand.h"
#include "Function.h"
#include "cfg_visitor.h"
#include "basicblock.h"

class IR {
public:
	IR();
	~IR();

	void addGlobalVar(std::shared_ptr<Register> var);
	void addFunction(std::shared_ptr<Function> func);
	void addStringConst(std::shared_ptr<StaticString> str);
	
	std::shared_ptr<Function> stringLength, substring, parseInt, ord, print, println;
	std::shared_ptr<Function> stringAdd, stringNEQ, stringEQ, stringLESS, stringLEQ, stringGREATER, stringGEQ;
	std::shared_ptr<Function> getInt, getString;
	std::shared_ptr<Function> toString, printInt, printlnInt;

	std::vector<std::shared_ptr<Function> > & getFunctions() { return functions; }
	std::vector<std::shared_ptr<Function> > & getBuiltInFunctions() { return builtInFunctions; }
	std::vector<std::shared_ptr<Register> > & getGlbVars() { return glbVars; }
	std::vector<std::shared_ptr<StaticString> > & getStringConstants() { return stringConstants; }

	bool isStringFunction(std::shared_ptr<Function> f) {
		auto n = f->getName();
		return n.length() > 7 && n.substr(0, 7) == "string.";
	}

	ACCEPT_CFG_VISITOR

private:
	std::vector<std::shared_ptr<Register> > glbVars;
	std::vector<std::shared_ptr<Function> > functions, builtInFunctions;
	std::vector<std::shared_ptr<StaticString> > stringConstants;
	
	std::shared_ptr<Function> 
		newBuiltInFunc(const std::string &name, std::shared_ptr<Type> retType = nullptr);
};

// return 
std::shared_ptr<Function> newFunction(const std::string &_name, std::shared_ptr<Type> retType);