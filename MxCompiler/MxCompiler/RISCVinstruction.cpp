#include "RISCVinstruction.h"

std::string B_type::toString()
{
	return op_to_string[op];
}

std::vector<std::shared_ptr<Register>> B_type::getUseReg()
{
	return { rs1, rs2};
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

std::string LoadAddr::toString()
{
	return "la";
}

std::shared_ptr<Register> LoadAddr::getDefReg()
{
	return rd;
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
