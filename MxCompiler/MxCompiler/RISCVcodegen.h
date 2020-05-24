#pragma once

#include "pch.h"
#include "cfg_visitor.h"
#include "instructionSelector.h"
#include "RISCVassembly.h"
/*
This class is an interface class for the toplevel MxCompiler class.
*/
class RISCVCodeGenerator : public CFG_Visitor {
public:
	RISCVCodeGenerator(std::shared_ptr<IR> _ir, std::ostream &_os) :os(_os), ir(_ir){}
	void generate();
	void emit();
private:
	std::shared_ptr<IR> ir;
	std::shared_ptr<RISCVProgram> riscv_program;
	std::ostream &os;

	std::unordered_map<std::shared_ptr<RISCVBasicBlock>, std::string> label;
	
	void emitFunction(std::shared_ptr<RISCVFunction> f);
	void emitBlock(std::shared_ptr<RISCVBasicBlock> b);
	void emitInstruction(std::shared_ptr<RISCVinstruction> i);

	std::string addr2String(std::shared_ptr<Address> addr);
};