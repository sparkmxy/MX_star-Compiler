#pragma once

#include "pch.h"
#include "RISCVinstruction.h"


class RISCVConfig {
public:
	static const std::string regNames[];
	static const std::string callerSaveRegNames[];
	static const std::string calleeSaveRegNames[];
};


class RISCVBasicBlock {
public:
	RISCVBasicBlock();
	void append(const std::shared_ptr<RISCVinstruction> &i);
private:

};

class RISCVFunction {
public:
	RISCVFunction(const std::string &_name) :name(_name) {}

	void setEntry(const std::shared_ptr<RISCVBasicBlock> &_entry) { entry = _entry; }
	std::shared_ptr<RISCVBasicBlock> getEntry() { return entry; }

	void setEixt(const std::shared_ptr<RISCVBasicBlock> &_exit) { exit = _exit; }
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

	std::shared_ptr<PhysicalRegister> operator [](const std::string &name){}
private:
	std::vector<std::shared_ptr<RISCVFunction> > functions, builtinFunctions;
};