#pragma once

#include "pch.h"
#include "configuration.h"
#include "vm.h"


namespace VMbuiltinFunctions {
	using Args = const std::vector<int> &;
	
	int print(Args args);
	int println(Args args);
	int string_length(Args args);
	int string_ord(Args args);
	int string_substring(Args args);
	int getString(Args args);
	int string_parseInt(Args args);

	int string_add(Args args);
	int string_eq(Args args);
	int string_neq(Args args);
	int string_less(Args args);
	int string_lesseq(Args args);
	int string_greater(Args args);
	int string_geq(Args args);

	int getInt(Args args);
	int printInt(Args args);
	int printIntln(Args args);
	int toString(Args args);

	static const std::unordered_map<std::string, std::function<int(Args)> > name2Func = {
	{"print", print}, {"println",println},
	{"string.length", string_length},{"string_ord",string_ord},
	{"string.substring",string_substring}, {"string_parseInt",string_parseInt},
	{"getString",getString}, {"string_add",string_add}, 
	{"string_eq",string_eq}, {"string_neq",string_neq},
	{"string_less",string_less}, {"string_leq",string_leq},
	{"string_greater",string_greater}, {"string_geq",string_geq},
	{"getInt",getInt}, {"printInt",printInt},
	{"printIntln",printIntln}, {"toString", toString}
	};
}

class IR_Interpreter {
public:
	IR_Interpreter(std::istream &_is) :is(_is) {}
	int run();
private:
	std::istream &is;
	MemoryManager M;

	std::unordered_map<std::string, std::shared_ptr<VMFunction> > functions;
	std::vector<std::string> globalVars;

	void parse();
	void parseFunction();
	std::shared_ptr<VMBasicBlock> parseBlock();
	std::shared_ptr<VMInstruction> parseInstruction();

	int executeFunction(std::string functionName, std::vector<int> args = std::vector<int>());
	int executeFunction(std::shared_ptr<VMFunction> f, std::vector<int> args);
	void executeInstruction(std::shared_ptr<VMInstruction> inst);

	bool nextchar() {
		char ch = is.get();
		while (isspace(ch)) ch = is.get();
		is.unget();
		return ch;
	}
};