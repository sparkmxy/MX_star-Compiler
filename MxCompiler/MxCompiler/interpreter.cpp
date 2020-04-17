#include "interpreter.h"

int IR_Interpreter::run()
{
	parse();
	return executeFunction("__bootstrap");
}


void IR_Interpreter::parse()
{
	std::string token;
	while (is >> token) {
		if (token[0] == '@') {
			if (nextchar() == '=') {
				// string constants
				is.get();
				std::string str, reg = token.substr(1,token.length() - 1);
				is >> str;
				str = str.substr(1, str.length() - 2);
				auto ptr = M.allocate_memory(str.length() + Configuration::SIZE_OF_INT);
				*reinterpret_cast<int *>(ptr) = str.length();
				std::memcpy(ptr + Configuration::SIZE_OF_INT, str.c_str(), str.length());
				M.setRegValue(reg, reinterpret_cast<int>(ptr));
			}
			// do we need to do anything for global variabels?
		}
		else if (token == "def") parseFunction();
		else throw Error("Invaild IR definition.");
	}
}

void IR_Interpreter::parseFunction()
{
	std::string name;
	is >> name;
	name = name.substr(1, name.length() - 2);
	std::unordered_set<std::string> args;
	while (nextchar() != '{') {
		std::string arg;
		is >> arg;
		args.insert(arg);
	}
	is.get();  // skip '{'
	auto f = std::make_shared<VMFunction>(name, args);
	
	while (nextchar() != '}') {
		auto b = parseBlock();
		f->appendBlock(b);
		if (b->isEntry()) f->setEntry(b);
	}
	is.get(); // skip '}'

	functions[name] = f;
}

std::shared_ptr<VMBasicBlock> IR_Interpreter::parseBlock()
{
	std::string label;
	is >> label;
	if (label[0] != '$') throw Error("invalid block");
	label = label.substr(1, label.length() - 2);
	auto b = std::make_shared<VMBasicBlock>(label);
	while(true) {
		char ch = nextchar();
		if (ch == '}' || ch == '$') break;
		b->appendInst(parseInstruction());
	}
	return b;
}

std::shared_ptr<VMInstruction> IR_Interpreter::parseInstruction()
{

}

int IR_Interpreter::executeFunction(std::string functionName, std::vector<int> args)
{
	auto it = VMbuiltinFunctions::name2Func.find(functionName);
	if (it != VMbuiltinFunctions::name2Func.end())
		it->second(args);
	executeFunction(functions[functionName], args);
	return 0;
}
