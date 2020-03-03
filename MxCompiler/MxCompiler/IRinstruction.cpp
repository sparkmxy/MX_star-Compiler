#include "IRinstruction.h"

void Quadruple::renameUseRegs(std::unordered_map<std::shared_ptr<Register>, std::shared_ptr<Register>>& table)
{
	if(Operand::isRegister(src1->category()))
		src1 = table[std::static_pointer_cast<Register>(src1)];
	if (src2 != nullptr && Operand::isRegister(src2->category()))
		src2 = table[std::static_pointer_cast<Register>(src2)];
	if (op == STORE && Operand::isRegister(dst->category())) // dst is also a use-register
		dst = table[std::static_pointer_cast<Register>(dst)];
	updateUseRegs();
}

void Quadruple::updateUseRegs()
{
	useRegs.clear();
	if (Operand::isRegister(src1->category()))
		useRegs.push_back(std::static_pointer_cast<Register>(src1));
	if (src2 != nullptr && Operand::isRegister(src1->category()))
		useRegs.push_back(std::static_pointer_cast<Register>(src2));
	if (op == STORE && Operand::isRegister(dst->category())) // dst is also a use-register
		useRegs.push_back(std::static_pointer_cast<Register>(dst));
}

void Branch::renameUseRegs(std::unordered_map<std::shared_ptr<Register>, std::shared_ptr<Register>>& table)
{
	if (Operand::isRegister(condition->category()))
		condition = table[std::static_pointer_cast<Register>(condition)];
	updateUseRegs();
}

void Branch::updateUseRegs()
{
	useRegs.clear();
	if(Operand::isRegister(condition->category()))
		useRegs.push_back(std::static_pointer_cast<Register>(condition));
}

void Call::renameUseRegs(std::unordered_map<std::shared_ptr<Register>, std::shared_ptr<Register>>& table)
{
	for (auto &arg : args)
		if (Operand::isRegister(arg->category()))
			arg = table[std::static_pointer_cast<Register>(arg)];
	if (Operand::isRegister(object->category()))
		object = table[std::static_pointer_cast<Register>(object)];
}

void Call::updateUseRegs()
{
	useRegs.clear();
	for (auto &arg : args)
		if (Operand::isRegister(arg->category()))
			useRegs.push_back(std::static_pointer_cast<Register>(arg));
	if (Operand::isRegister(object->category()))
		useRegs.push_back(std::static_pointer_cast<Register>(object));
}

std::shared_ptr<Register> Call::getDefReg()
{
	if (result != nullptr) return std::static_pointer_cast<Register>(result);
}

void Call::setDefReg(std::shared_ptr<Register> _defReg)
{
	result = _defReg;
}

void Return::renameUseRegs(std::unordered_map<std::shared_ptr<Register>, std::shared_ptr<Register>>& table)
{
	if (Operand::isRegister(value->category()))
		value = table[std::static_pointer_cast<Register>(value)];
	updateUseRegs();
}

void Return::updateUseRegs()
{
	useRegs.clear();
	if (Operand::isRegister(value->category())) 
		useRegs.push_back(std::static_pointer_cast<Register>(value));
}

void Malloc::renameUseRegs(std::unordered_map<std::shared_ptr<Register>, std::shared_ptr<Register>>& table)
{
	if (Operand::isRegister(size->category()))
		size = table[std::static_pointer_cast<Register>(size)];
	updateUseRegs();
}

void Malloc::updateUseRegs()
{
	useRegs.clear();
	if (Operand::isRegister(size->category()))
		useRegs.push_back(std::static_pointer_cast<Register>(size));
}


std::shared_ptr<Register> PhiFunction::getDefReg()
{
	return dst;
}

void PhiFunction::setDefReg(std::shared_ptr<Register> _defReg)
{
	dst = _defReg;
}
