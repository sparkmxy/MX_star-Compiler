#include "RISCVinstruction.h"

std::string B_type::toString()
{
	return op_to_string[op];
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

std::string I_type::toString()
{
	return op_to_string[op];
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
	if (rs1 == reg) rs1 = reg;
}

void I_type::updateDefReg(std::shared_ptr<Register> new_reg)
{
	rd = new_reg;
}

std::string R_type::toString()
{
	return op_to_string[op];
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
	rd == new_reg;
}

std::string MoveAssembly::toString()
{
	return "mov";
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
	return "jump";
}

std::string LoadImm::toString()
{
	return "li";
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
	return "la";
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
	return "load";
}

std::vector<std::shared_ptr<Register>> Load::getUseReg()
{
	if (Operand::isRegister(addr->category())) {
		auto reg = std::static_pointer_cast<Register>(addr);
		if (reg->isGlobal()) return {};
		return { reg };
	}
	if (addr->category() == Operand::STACK)
		return { std::static_pointer_cast<StackLocation>(addr)->getBase() };
	throw Error("Nobody knows exception better than me.");
}

std::shared_ptr<Register> Load::getDefReg()
{
	return rd;
}

void Load::updateUseReg(std::shared_ptr<Register> reg, std::shared_ptr<Register> new_reg)
{
	if (reg == addr) addr = reg;
}

void Load::updateDefReg(std::shared_ptr<Register> new_reg)
{
	rd = new_reg;
}

std::string Store::toString()
{
	return "store";
}

std::vector<std::shared_ptr<Register>> Store::getUseReg()
{
	std::vector<std::shared_ptr<Register>> ret = { rs };

	if (Operand::isRegister(addr->category())) {
		if (!std::static_pointer_cast<Register>(addr)->isGlobal())
			ret.push_back(std::static_pointer_cast<Register>(addr));
	}
	else if (addr->category() == Operand::STACK)
		ret.push_back(std::static_pointer_cast<StackLocation>(addr)->getBase());
	return ret;
}

void Store::updateUseReg(std::shared_ptr<Register> reg, std::shared_ptr<Register> new_reg)
{
	if (addr == reg) addr = new_reg;
	if (rs == reg) rs = new_reg;
} 
