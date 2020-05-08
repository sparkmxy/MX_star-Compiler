#pragma once

#include "cfg_visitor.h"

class X86CodeGenerator : public CFG_Visitor {
public:
	X86CodeGenerator(std::shared_ptr<IR> _ir, std::ostream &_os) :ir(_ir), os(_os) {}
	void run();
private:
	std::shared_ptr<IR> ir;
	std::ostream &os;
};