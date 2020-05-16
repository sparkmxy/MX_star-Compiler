#pragma once

#include "cfg_visitor.h"
#include "RISCVassembly.h"
/*
This class is an interface class for the toplevel MxCompiler class.
*/
class RISCVCodeGenerator : public CFG_Visitor {
public:
	RISCVCodeGenerator(std::shared_ptr<IR> _ir) :ir(_ir){}
	void generate();
	void emit(std::ostream &os);
private:
	std::shared_ptr<IR> ir;
	std::shared_ptr<RISCVProgram> riscv_program;
	// Data structures that hold the RISCV assembly
};