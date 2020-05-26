#pragma once

#include "pch.h"
#include "IRinstruction.h"
#include "cfg_visitor.h"

class BasicBlock;
class DominatorTree;
/*
Class: Function
The main component of a function module is its control flow graph. 
Warning: Do not call the Constructor directly, use newFunction()(c.f. ir.h) instead.
*/
class Function : public std::enable_shared_from_this<Function>{
public:
	Function(const std::string &_name, std::shared_ptr<Type> retType) 
		: name(_name){
		_isVoid = (retType == nullptr);
	}

	void appendReturnInstr(std::shared_ptr<Return> ret);
	std::vector<std::shared_ptr<Return> > &getReturnIntrs() { return retInstrs; }

	void appendArg(std::shared_ptr<Register> arg);
	std::vector<std::shared_ptr<Register> > &getArgs() { return args; }
	// getters/setters
	std::string getName() { return name; }

	std::shared_ptr<BasicBlock> getEntry() { return entry; }
	void setEntry(const std::shared_ptr<BasicBlock> &_entry) { entry = _entry; }

	std::shared_ptr<BasicBlock> getExit() { return exit; }
	void setExit(const std::shared_ptr<BasicBlock> &_exit) { exit = _exit; }

	std::shared_ptr<Operand> getObjRef() { return objRef; }
	void setObjRef(const std::shared_ptr<Operand> &ref) { objRef = ref; }

	void appendBlocktoList(std::shared_ptr<BasicBlock> b) { blocks.push_back(b); }
	std::vector<std::shared_ptr<BasicBlock> > &getBlockList() { return blocks;}
	//for SSA

	std::shared_ptr<DominatorTree> getDT() { return dt; }
	void setDT(std::shared_ptr<DominatorTree> _dt) { dt = _dt; }
	void append_var(std::shared_ptr<VirtualReg> reg) { vars.insert(reg); }
	std::unordered_set<std::shared_ptr<VirtualReg> > &getVars() { return vars; }

	bool isVoid() { return _isVoid; }

	ACCEPT_CFG_VISITOR
private:

	bool _isVoid;
	std::shared_ptr<BasicBlock> entry, exit;
	std::string name;

	std::vector<std::shared_ptr<Return> > retInstrs;
	std::vector<std::shared_ptr<Register> > args;

	std::shared_ptr<Operand> objRef;

	//for SSA
	std::shared_ptr<DominatorTree> dt;
	//return a list of block in this function module in DFS order on dominance tree;
	std::vector<std::shared_ptr<BasicBlock> > blocks;
	std::unordered_set<std::shared_ptr<VirtualReg> > vars;
};