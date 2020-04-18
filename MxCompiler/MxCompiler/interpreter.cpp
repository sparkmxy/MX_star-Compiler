#include "interpreter.h"

void IR_Interpreter::run()
{
	parse();
	os << "ir parse done." << std::endl;
	int exitCode = executeFunction("__bootstrap");
	os << "exit with " << exitCode << std::endl;
}


int IR_Interpreter::getRegVal(const std::string & name, std::unordered_map<std::string, int>& local)
{
	if(name[0] == '#'){ // immediate
		std::stringstream ss(name.substr(1, name.length() - 1));
		int v;
		ss >> v;
		return v;
	}
	auto it = local.find(name);
	if (it != local.end()) return it->second;
	return M.getRegValue(name);
}

void IR_Interpreter::setRegVal(const std::string & name, std::unordered_map<std::string, int>& local, int v)
{
	auto it = local.find(name);
	if (it != local.end()) it->second = v;
	M.setRegValue(name, v);
}

void IR_Interpreter::parse()
{
	std::string token;
	while (is >> token) {
		if (token[0] == '@') {
			if (nextchar() == '=') {
				// string constants
				is.get();  // skip '='
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
	std::vector<std::string> args;
	while (nextchar() != '{') {
		std::string arg;
		is >> arg;
		args.push_back(arg);
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
	std::string op, dst, src1, src2;

	is >> op;
	if (op == "call") {
		std::vector<std::string> args;
		is >> dst >> src1;  // function name and reg for return  value; note that src1 could be "null"

		while (nextchar() != '{') {
			is >> src2;
			args.push_back(src2);
		}
		return std::make_shared<VMInstruction>(op, dst, src1, "", args);
	}
	if (op == "ret" || op == "jmp") {
		is >> dst;
		return std::make_shared<VMInstruction>(op, dst);
	}
	else if(op == "inv" || op == "neg" || op == "mov" || op == "load" || op == "store") {
		is >> dst >> src1;
		return std::make_shared<VMInstruction>(op, dst, src1);
	}
	else {
		is >> dst >> src1 >> src2;
		return std::make_shared<VMInstruction>(op, dst, src1, src2);
	}
}

int IR_Interpreter::executeFunction(std::string functionName, std::vector<int> args)
{
	auto it = name2Func.find(functionName);
	if (it != name2Func.end())
		return it->second(*this,args);
	return executeFunction(functions[functionName], args);
}

int IR_Interpreter::executeFunction(std::shared_ptr<VMFunction> f, std::vector<int> args)
{
	auto argMap = f->getArgMap(args);
	std::shared_ptr<VMBasicBlock> curBlock = f->getEntry();
	while (true) {
		auto instructions = curBlock->getInstructions();
		for (auto &inst : instructions) {
			if (inst->op == "ret") {
				return getRegVal(inst->dst, argMap);
			}
			else if (inst->op == "jmp") {
				curBlock = f->getBlockByLabel(inst->dst);
			}
			else if (inst->op == "br") {
				curBlock = f->getBlockByLabel(
					getRegVal(inst->dst,argMap) ? inst->src1 : inst->src2
				);
			}
			else executeInstruction(inst, argMap);
		}
	}
	return 0;
}

void IR_Interpreter::executeInstruction(std::shared_ptr<VMInstruction> inst, std::unordered_map<std::string, int> &localRegs)
{
#ifdef SHOW_LOG
	std::clog << inst->toString() << '\n';
#endif // SHOW_LOG

	if (inst->op == "add")
		setRegVal(inst->dst, localRegs, getRegVal(inst->src1, localRegs) + getRegVal(inst->src2, localRegs));
	else if (inst->op == "sub")
		setRegVal(inst->dst, localRegs, getRegVal(inst->src1, localRegs) - getRegVal(inst->src2, localRegs));
	else if (inst->op == "mul")
		setRegVal(inst->dst, localRegs, getRegVal(inst->src1, localRegs) * getRegVal(inst->src2, localRegs));
	else if (inst->op == "div")
		setRegVal(inst->dst, localRegs, getRegVal(inst->src1, localRegs) / getRegVal(inst->src2, localRegs));
	else if (inst->op == "mod")
		setRegVal(inst->dst, localRegs, getRegVal(inst->src1, localRegs) % getRegVal(inst->src2, localRegs));
	else if (inst->op == "shl")
		setRegVal(inst->dst, localRegs, getRegVal(inst->src1, localRegs) << getRegVal(inst->src2, localRegs));
	else if (inst->op == "shr")
		setRegVal(inst->dst, localRegs, getRegVal(inst->src1, localRegs) >> getRegVal(inst->src2, localRegs));
	else if (inst->op == "seq")
		setRegVal(inst->dst, localRegs, getRegVal(inst->src1, localRegs) == getRegVal(inst->src2, localRegs));
	else if (inst->op == "neq")
		setRegVal(inst->dst, localRegs, getRegVal(inst->src1, localRegs) != getRegVal(inst->src2, localRegs));
	else if (inst->op == "slt")
		setRegVal(inst->dst, localRegs, getRegVal(inst->src1, localRegs) < getRegVal(inst->src2, localRegs));
	else if (inst->op == "sle")
		setRegVal(inst->dst, localRegs, getRegVal(inst->src1, localRegs) <= getRegVal(inst->src2, localRegs));
	else if (inst->op == "sgt")
		setRegVal(inst->dst, localRegs, getRegVal(inst->src1, localRegs) > getRegVal(inst->src2, localRegs));
	else if (inst->op == "sge")
		setRegVal(inst->dst, localRegs, getRegVal(inst->src1, localRegs) >= getRegVal(inst->src2, localRegs));
	else if (inst->op == "and")
		setRegVal(inst->dst, localRegs, getRegVal(inst->src1, localRegs) & getRegVal(inst->src2, localRegs));
	else if (inst->op == "or")
		setRegVal(inst->dst, localRegs, getRegVal(inst->src1, localRegs) | getRegVal(inst->src2, localRegs));
	else if (inst->op == "xor")
		setRegVal(inst->dst, localRegs, getRegVal(inst->src1, localRegs) ^ getRegVal(inst->src2, localRegs));
	else if (inst->op == "inv")
		setRegVal(inst->dst, localRegs, !getRegVal(inst->src1, localRegs));
	else if (inst->op == "neg")
		setRegVal(inst->dst, localRegs, -getRegVal(inst->src1, localRegs));
	else if (inst->op == "mov")
		setRegVal(inst->dst, localRegs, getRegVal(inst->src1, localRegs));
	else if (inst->op == "load")
		setRegVal(inst->dst, localRegs, M.load(getRegVal(inst->src1,localRegs)));
	else if (inst->op == "store")
		M.store(getRegVal(inst->dst, localRegs), getRegVal(inst->src1, localRegs));
	else if (inst->op == "call") {
		std::vector<int> argv;
		for (auto &reg : inst->args) argv.push_back(getRegVal(reg, localRegs));
		if (inst->src1 == "null") // no return value
			executeFunction(inst->dst, argv);
		else
			setRegVal(inst->src1, localRegs, executeFunction(inst->dst, argv));
	}
	else throw Error("What the fuck are you doing here!");
}

int IR_Interpreter::print(Args args)
{
	os << fetchString(args[0]);
	return 0;
}

int IR_Interpreter::println(Args args)
{
	os << fetchString(args[0]) << '\n';
	return 0;
}

int IR_Interpreter::string_length(Args args)
{
	return M.load(args[0]);
}

int IR_Interpreter::string_ord(Args args)
{
	return M.loadByte(args[0] + Configuration::SIZE_OF_INT);
}

int IR_Interpreter::string_substring(Args args)
{
	auto str = fetchString(args[0]).substr(args[1], args[2] - args[1] + 1);
	return storeString(str);
}

int IR_Interpreter::getString(Args args)
{
	std::string str;
	user >> str;
	return storeString(str);
}

int IR_Interpreter::string_parseInt(Args args)
{
	std::string str = fetchString(args[0]);
	std::vector<int> V;
	std::stringstream ss(str);
	int x;
	while (ss >> x) V.push_back(x);
	Byte *ptr = M.allocate_memory((V.size() + 1) * Configuration::SIZE_OF_INT);
	M.store((int)ptr, V.size());
	for (Byte *cur = ptr + Configuration::SIZE_OF_INT, i = 0; i < V.size(); i++) {
		M.store((int)cur, V[i]);
		cur += Configuration::SIZE_OF_INT;
	}
	return (int)ptr;
}

int IR_Interpreter::string_add(Args args)
{
	return storeString(fetchString(args[0]) + fetchString(args[1]));
}

int IR_Interpreter::string_eq(Args args)
{
	return fetchString(args[0]) == fetchString(args[1]);
}

int IR_Interpreter::string_neq(Args args)
{
	return fetchString(args[0]) != fetchString(args[1]);
}

int IR_Interpreter::string_less(Args args)
{
	return fetchString(args[0]) < fetchString(args[1]);
}

int IR_Interpreter::string_leq(Args args)
{
	return fetchString(args[0]) <= fetchString(args[1]);
}

int IR_Interpreter::string_greater(Args args)
{
	return fetchString(args[0]) > fetchString(args[1]);
}

int IR_Interpreter::string_geq(Args args)
{
	return fetchString(args[0]) >= fetchString(args[1]);
}

int IR_Interpreter::getInt(Args args)
{
	int x;
	user >> x;
	return x;
}

int IR_Interpreter::printInt(Args args)
{
	os << args[0];
	return 0;
}

int IR_Interpreter::printIntln(Args args)
{
	os << args[0] << '\n';
	return 0;
}

int IR_Interpreter::toString(Args args)
{
	return storeString(std::to_string(args[0]));
}

std::string IR_Interpreter::fetchString(int addr)
{
	int l = M.load(addr);  // length
	auto ptr = reinterpret_cast<char *>(addr) + Configuration::SIZE_OF_INT;
	char *str = new char[l + 1];
	std::memcpy(str, ptr, l);
	str[l] = '\0';
	auto ret =  std::string(str);
	delete[] str;
	return ret;
}

int IR_Interpreter::storeString(std::string str)
{
	Byte *ptr = M.allocate_memory(str.length() + Configuration::SIZE_OF_INT);
	M.store((int)ptr, str.length());
	std::memcpy(ptr + Configuration::SIZE_OF_INT, str.c_str(), str.length());
	return (int)ptr;
}
