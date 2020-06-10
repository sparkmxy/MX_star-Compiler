#include "RISCVinstruction.h"


int RISCVCNT = 0;

const std::string B_type::op_to_string[] = {"beq", "bne", "ble", "bge", "blt", "bgt"};

std::string B_type::toString()
{
	return op_to_string[op] + "\t" + rs1->getName() + ", " + rs2->getName(); // no target Name
}

std::vector<std::shared_ptr<Register>> B_type::getUseReg()
{
	return { rs1, rs2};
}

void B_type::updateUseReg(std::shared_ptr<Register> reg, std::shared_ptr<Register> new_reg)
{
	if (rs1 == reg) rs1 = new_reg;
	if (rs2 == reg) rs2 = new_reg;
}

const std::string I_type::op_to_string[] = {"addi","xori","ori","andi","slti","sltiu","slli","srai"};

std::string I_type::toString()
{
	//if (rs1->getName() == "")
	//	throw Error("holy shit");
	return op_to_string[op] + "\t" + rd->getName() + ", " + rs1->getName() + ", " + std::to_string(imm->getValue());
}

std::vector<std::shared_ptr<Register>> I_type::getUseReg()
{
	return {rs1};
}

std::shared_ptr<Register> I_type::getDefReg()
{
	return rd;
}

void I_type::updateUseReg(std::shared_ptr<Register> reg, std::shared_ptr<Register> new_reg)
{
	if (rs1 == reg) rs1 = new_reg;
}

void I_type::updateDefReg(std::shared_ptr<Register> new_reg)
{
	rd = new_reg;
}

const std::string R_type::op_to_string[] = {
	"add", "sub", "mul", "div", "rem", "sll", "sra", "slt", "sltu", 
	"and", "or", "xor"
};

std::string R_type::toString()
{
	return op_to_string[op] + "\t" + rd->getName() + ", " + rs1->getName() + ", " + rs2->getName();
}

std::vector<std::shared_ptr<Register>> R_type::getUseReg()
{
	return { rs1, rs2 };
}

std::shared_ptr<Register> R_type::getDefReg()
{
	return rd;
}

void R_type::updateUseReg(std::shared_ptr<Register> reg, std::shared_ptr<Register> new_reg)
{
	if (reg == rs1) rs1 = new_reg;
	if (reg == rs2) rs2 = new_reg;
}

void R_type::updateDefReg(std::shared_ptr<Register> new_reg)
{
	rd = new_reg;
}

std::string MoveAssembly::toString()
{
	return "mv\t" + rd->getName() + ", " + rs1->getName();
}

std::vector<std::shared_ptr<Register>> MoveAssembly::getUseReg()
{
	return { rs1 };
}

std::shared_ptr<Register> MoveAssembly::getDefReg()
{
	return rd;
}

void MoveAssembly::updateUseReg(std::shared_ptr<Register> reg, std::shared_ptr<Register> new_reg)
{
	if (rs1 == reg) rs1 = new_reg;
}

void MoveAssembly::updateDefReg(std::shared_ptr<Register> new_reg)
{
	rd = new_reg;
}

std::string JumpAssembly::toString()
{
	return "j\t";  // need to add targetblock
}

std::string LoadImm::toString()
{
	return "li\t" + rd->getName() + ", " + std::to_string(imm->getValue());
}

std::shared_ptr<Register> LoadImm::getDefReg()
{
	return rd;
}

void LoadImm::updateDefReg(std::shared_ptr<Register> new_reg)
{
	rd = new_reg;
}

std::string LoadAddr::toString()
{
	return "la\t" + rd->getName() + ", "; // what should I do here?
}

std::shared_ptr<Register> LoadAddr::getDefReg()
{
	return rd;
}

void LoadAddr::updateDefReg(std::shared_ptr<Register> new_reg)
{
	rd = new_reg;
}

std::string Load::toString()
{
	std::string op = "lw";
	if (size == 2) op = "lh";
	if (size == 1) op = "lb";
	return op + "\t" + rd->getName(); // need to print the addr;
}

std::vector<std::shared_ptr<Register>> Load::getUseReg()
{
	if (addr->category() == Operand::STACK)
		return { std::static_pointer_cast<StackLocation>(addr)->getSp()};
	if(addr->category() == Operand::ADDR){
		auto reg = std::static_pointer_cast<BaseOffsetAddr>(addr)->getBase();
		if (reg->isGlobal()) return {};
		return { reg };
	}
	throw Error("Nobody knows exception better than me.");
}

std::shared_ptr<Register> Load::getDefReg()
{
	return rd;
}

void Load::updateUseReg(std::shared_ptr<Register> reg, std::shared_ptr<Register> new_reg)
{
	if (addr->category() == Operand::ADDR) {
		auto a = std::static_pointer_cast<BaseOffsetAddr>(addr);
		if (a->getBase() == reg) a->setBase(new_reg);
	}
}

void Load::updateDefReg(std::shared_ptr<Register> new_reg)
{
	rd = new_reg;
}

std::string Store::toString()
{
	std::string op = "sw";
	if (size == 2) op = "sh";
	if (size == 1) op = "sb";

	return op + "\t" + rs->getName(); // need to add addr
}

std::vector<std::shared_ptr<Register>> Store::getUseReg()
{
	std::vector<std::shared_ptr<Register>> ret = { rs };

	if (addr->category() == Operand::ADDR) {
		auto a = std::static_pointer_cast<BaseOffsetAddr>(addr);
		if (!a->getBase()->isGlobal()) ret.push_back(a->getBase());
	}
	else if (addr->category() == Operand::STACK)
		ret.push_back(std::static_pointer_cast<StackLocation>(addr)->getSp());
	return ret;
}

std::shared_ptr<Register> Store::getDefReg()
{
	return rt;
}

void Store::updateUseReg(std::shared_ptr<Register> reg, std::shared_ptr<Register> new_reg)
{
	if (rs == reg) rs = new_reg;
	if (addr->category() == Operand::ADDR) {
		auto a = std::static_pointer_cast<BaseOffsetAddr>(addr);
		if (a->getBase() == reg) a->setBase(new_reg);
	}
}
void Store::updateDefReg(std::shared_ptr<Register> new_reg)
{
	if (rt != nullptr) rt = new_reg;
	else throw Error("what the fuck are you doing here?");
}


std::string CallAssembly::toString()
{
	return "call\t"; // need to print function name
}

RISCVinstruction::RISCVinstruction(std::weak_ptr<RISCVBasicBlock> b, Category _c) :residingBlk(b), c(_c) {
	
	RISCVCNT++;
}
