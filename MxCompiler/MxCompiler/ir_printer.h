#pragma once

#include "pch.h"
#include "IR.h"
#include "cfg_visitor.h"

class IR_Printer : public CFG_Vistor{
public:
	IR_Printer(std::shared_ptr<IR> _ir, std::ostream &_os = std::cerr) 
		:ir(_ir),os(_os){}
private:
	std::shared_ptr<IR> ir;
	std::ostream &os;
};