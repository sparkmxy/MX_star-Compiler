#include "RISCVassembly.h"

const std::string RISCVConfig::regNames[] = { 
		"zero", "ra", "sp", "gp", "tp", 
		"t0", "t1", "t2", "s0", "s1",
		"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
		"s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",
		"t3", "t4", "t5", "t6" 
};

const std::string RISCVConfig::calleeSaveRegNames[] = { 
	"s0", "s1", "s2", "s3", "s4", "s5"," s6", "s7", "s8", "s9", "s10", "s11" 
};

const std::string RISCVConfig::callerSaveRegNames[] = {
	"ra", 
	"t0", "t1", "t2", 
	"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", 
	"t3","t4","t5","t6"   // temporaries
};


RISCVBasicBlock::RISCVBasicBlock()
{
}
