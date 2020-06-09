#pragma once
#include "pch.h"
#include "configuration.h"

using Byte = char;

struct VMInstruction {
	/*
		Instructions:
		br, jmp, ret,
		add, sub, mul, div, mod, shl, slr, 
		seq, sne, slt, sle, sgt, sge
		and, or, xor, inv, neg, 
		load, store, mov
	*/
	VMInstruction(const std::string &_op, std::string _dst, std::string _src1 = "",
		std::string _src2 = "", std::vector<std::string> _args = std::vector<std::string>())
		:op(_op), src1(_src1), src2(_src2), dst(_dst), args(_args) {}

	std::string op, src1, src2, dst;
	// for br dst = condition, src1 = target_block1, src2 = target_block2
	std::vector<std::string> args; // for call instrction

	std::string toString() { 
		std::string arg = "";
		for (auto &str : args) arg += " " + str;
		return op + " " + dst + " " + src1 + " " + src2 + arg; 
		
	}
};

class VMBasicBlock {
public:
	VMBasicBlock(const std::string &_label) :label(_label) {}
	std::string getLabel() { return label; }

	void appendInst(std::shared_ptr<VMInstruction> inst) { instructions.push_back(inst); }
	std::vector<std::shared_ptr<VMInstruction> > &getInstructions() { return instructions; }

	bool isEntry() { return label.substr(0, 5) == "entry"; }
private:   // make all public?   
	std::string label;
	std::vector<std::shared_ptr<VMInstruction> > instructions;
};


class VMFunction {
public:
	VMFunction(const std::string &_name, std::vector<std::string> _args)
		: name(_name), args(_args) {}

	std::string getName() { return name; }
	void setEntry(std::shared_ptr<VMBasicBlock> _entry) { entry = _entry; }
	std::shared_ptr<VMBasicBlock> getEntry() { return entry; }

	std::map<std::string, int> getArgMap(std::vector<int> V) {
		std::map<std::string, int> ret;
		for (int i = 0; i < V.size(); i++) ret[args[i]] = V[i];
		return ret;
	}

	std::shared_ptr<VMBasicBlock> getBlockByLabel(const std::string &label) { return blocks[label]; }
	void appendBlock(std::shared_ptr<VMBasicBlock> b) { blocks[b->getLabel()] = b; }
private:
	std::string name;
	std::shared_ptr<VMBasicBlock> entry;
	std::map<std::string, std::shared_ptr<VMBasicBlock> > blocks;
	std::vector<std::string> args;
};

class MemoryManager {
public:
	static const int REG_SIZE = 4;

	MemoryManager() :poolSize(1024), used(0){
		mem = new Byte[poolSize];
		memPools.push_back(mem);
	}

	~MemoryManager() {
		for (auto mem : memPools) delete[] mem;
	}

	int getRegValue(const std::string &name);
	void setRegValue(const std::string &name, int v);

	// the address should be interpreted as Byte*
	int load(int  addr);
	void store(int addr,int v);
	Byte loadByte(int addr);

	Byte *allocate_memory(int size);

private:
	Byte *mem;
	std::vector<Byte *> memPools;
	int poolSize, used;

	std::map<std::string, int> regs;  // registers
};