#pragma once

#include "pch.h"
#include "configuration.h"
#include "vm.h"

// #define SHOW_LOG

class IR_Interpreter {
public:
	IR_Interpreter(std::istream &_is, std::ostream &_os = std::cout)
		:is(_is), os(_os), user(std::cin) {}

	void run();
private:
	std::istream &is, &user;
	std::ostream &os;
	MemoryManager M;

	std::unordered_map<std::string, std::shared_ptr<VMFunction> > functions;
	std::vector<std::string> globalVars;

	// The "register" could be a immediate
	int getRegVal(const std::string &name, std::unordered_map<std::string, int> &local);
	void setRegVal(const std::string &name, std::unordered_map<std::string, int> &local, int v);

	void parse();
	void parseFunction();
	std::shared_ptr<VMBasicBlock> parseBlock();
	std::shared_ptr<VMInstruction> parseInstruction();

	int executeFunction(std::string functionName, std::vector<int> args = std::vector<int>());
	int executeFunction(std::shared_ptr<VMFunction> f, std::vector<int> args);
	
	// return whether need to jump to some other block
	void executeInstruction(std::shared_ptr<VMInstruction> inst, 
		std::unordered_map<std::string, int> &args);

	char nextchar() {
		char ch = is.get();
		while (isspace(ch)) ch = is.get();
		is.unget();
		return ch;
	}

	std::stack<std::string> lastBlockName, curblockName;
	int getPhiVal(std::vector<std::string> &V, std::unordered_map<std::string, int> &local);
	// builtin function
public:
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
	int string_leq(Args args);
	int string_greater(Args args);
	int string_geq(Args args);

	int getInt(Args args);
	int printInt(Args args);
	int printIntln(Args args);
	int toString(Args args);

private: // helper functions
	std::string fetchString(int addr); 
	int storeString(std::string str);
};

static const std::unordered_map<std::string, std::function<int(IR_Interpreter &, IR_Interpreter::Args)> > name2Func = {
	{"print", &IR_Interpreter::print}, 
	{"println",&IR_Interpreter::println},
	{"string.length", &IR_Interpreter::string_length},
	{"string_ord",&IR_Interpreter::string_ord},
	{"string.substring",&IR_Interpreter::string_substring}, 
	{"string_parseInt",&IR_Interpreter::string_parseInt},
	{"getString",&IR_Interpreter::getString},
	{"string_add",&IR_Interpreter::string_add},
	{"string_eq",&IR_Interpreter::string_eq}, 
	{"string_neq",&IR_Interpreter::string_neq},
	{"string_less",&IR_Interpreter::string_less},
	{"string_leq",&IR_Interpreter::string_leq},
	{"string_greater",&IR_Interpreter::string_greater}, 
	{"string_geq",&IR_Interpreter::string_geq},
	{"getInt",&IR_Interpreter::getInt}, 
	{"printInt",&IR_Interpreter::printInt},
	{"printIntln",&IR_Interpreter::printIntln}, 
	{"toString", &IR_Interpreter::toString}
};