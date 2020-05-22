#pragma once

#include "pch.h"
#include "RISCVinstruction.h"


class RISCVConfig {
public:
	static const std::vector<std::string> physicalRegNames;
	static const std::vector<std::string> callerSaveRegNames;
	static const std::vector<std::string> calleeSaveRegNames;
};


class RISCVBasicBlock :public std::enable_shared_from_this<RISCVBasicBlock> {
public:
	RISCVBasicBlock(std::weak_ptr<RISCVFunction> _f, std::string _label) 
		: label(_label), func(_f) {}

	void append(const std::shared_ptr<RISCVinstruction> &i);

	std::string getLabel() { return label; }

	std::shared_ptr<RISCVinstruction> getFront() { return front; }
	void setFront(std::weak_ptr<RISCVinstruction> i) { front = i.lock; }

	std::shared_ptr<RISCVinstruction> getBack() { return back.lock(); }
	void setBack(std::weak_ptr<RISCVinstruction> i) { back = i; }

	std::shared_ptr<RISCVFunction> getResidingFunction() { return func.lock(); }

	std::unordered_set<std::weak_ptr<RISCVBasicBlock> > &getToBlocks() { return to; }
	std::unordered_set<std::weak_ptr<RISCVBasicBlock> > &getFromBlocks() { return from; }

	void insertToBlock(std::weak_ptr<RISCVBasicBlock> b) { to.insert(b); }
	void insertFromBlock(std::weak_ptr<RISCVBasicBlock> b) { from.insert(b); }

private:
	std::string label;
	std::shared_ptr<RISCVinstruction> front;
	std::weak_ptr<RISCVinstruction> back;
	std::weak_ptr<RISCVFunction> func;

	std::unordered_set<std::weak_ptr<RISCVBasicBlock> > from, to;
};

class RISCVFunction {
public:
	RISCVFunction(const std::string &_name) :name(_name) {}

	void setEntry(const std::shared_ptr<RISCVBasicBlock> &_entry) { entry = _entry; }
	std::shared_ptr<RISCVBasicBlock> getEntry() { return entry; }

	void setExit(const std::shared_ptr<RISCVBasicBlock> &_exit) { exit = _exit; }
	std::shared_ptr<RISCVBasicBlock> getExit() { return exit; }

	std::vector<std::shared_ptr<RISCVBasicBlock> > &getBlockList() { return blocks; }
	void appendBlock(std::shared_ptr<RISCVBasicBlock> b) { blocks.push_back(b); }

	std::string getName() { return name; }
private:
	std::string name;
	std::vector<std::shared_ptr<RISCVBasicBlock> > blocks;

	std::shared_ptr<RISCVBasicBlock> entry, exit;
};


class RISCVProgram {
public:
	RISCVProgram();
	void print(std::ostream &os);

	void appendFunction(std::shared_ptr<RISCVFunction> f) { functions.push_back(f); }
	
	std::vector<std::shared_ptr<RISCVFunction> > &getFunctions() { return functions; }

	std::shared_ptr<PhysicalRegister> operator [](const std::string &name){
		auto it = physicalRegs.find(name);
		if (it == physicalRegs.end()) throw Error("invalid physical register name.");
		return it->second;
	}

	std::shared_ptr<RISCVFunction> getMallocFunction() { return mallocFunc; }

	void resetPrecoloredRegs();
private:
	std::vector<std::shared_ptr<RISCVFunction> > functions, builtinFunctions;

	std::unordered_map<std::string, std::shared_ptr<PhysicalRegister> > physicalRegs;

	std::shared_ptr<RISCVFunction> mallocFunc;
};

void removeRISCVinstruction(std::shared_ptr<RISCVinstruction> i);