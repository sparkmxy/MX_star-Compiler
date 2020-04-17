#pragma once
#include "pch.h"
#include "configuration.h"

using Byte = char;

struct VMInstruction {
	VMInstruction(const std::string &_op, const std::string &_src1, const std::string &_src2,
		const std::string &_dst, std::vector<std::string> _args)
		:op(_op), src1(_src1), src2(_src2), dst(_dst), args(_args) {}

	std::string op, src1, src2, dst;
	// for br dst = condition, src1 = target_block1, src2 = target_block2
	std::vector<std::string> args; // for call instrction
};

class VMBasicBlock {
public:
	VMBasicBlock(const std::string &_label) :label(_label) {}
	std::string getLabel() { return label; }
	void appendInst(std::shared_ptr<VMInstruction> inst) { instructions.push_back(inst); }

	bool isEntry() { return label.substr(0, 5) == "entry"; }
private:   // make all public?   
	std::string label;
	std::vector<std::shared_ptr<VMInstruction> > instructions;
};


class VMFunction {
public:
	VMFunction(const std::string &_name, std::unordered_set<std::string> _args)
		: name(_name), args(_args) {}

	std::string getName() { return name; }
	void setEntry(std::shared_ptr<VMBasicBlock> _entry) { entry = _entry; }
	std::shared_ptr<VMBasicBlock> getEntry() { return entry; }

	void appendBlock(std::shared_ptr<VMBasicBlock> b) { blocks[b->getLabel()] = b; }
private:
	std::string name;
	std::shared_ptr<VMBasicBlock> entry;
	std::unordered_map<std::string, std::shared_ptr<VMBasicBlock> > blocks;
	std::unordered_set<std::string> args;
};

class MemoryManager {
public:
	static const int REG_SIZE = 4;

	MemoryManager() :poolSize(1024), used(0){
		mem = new Byte[poolSize];
	}

	~MemoryManager() {
		delete[] mem;
	}
	int getRegValue(const std::string &reg);
	void setRegValue(const std::string &reg, int v);

	int load(int addr, const std::string &reg);
	void store(int addr, const std::string &reg);

	Byte *allocate_memory(int size);
private:
	Byte *mem;
	int poolSize, used;

	std::unordered_map<std::string, int> regs;  // registers

	void doubleSpace();
};