#include "IRinstruction.h"

void Quadruple::renameUseRegs(std::unordered_map<std::shared_ptr<Register>, std::shared_ptr<Register>>& table)
{
	updateRegister(src1, table);
	if (src2 != nullptr) updateRegister(src2, table);
	if (op == STORE) // dst is also a use-register
		updateRegister(dst, table);
	updateUseRegs();
}

void Quadruple::updateUseRegs()
{
	useRegs.clear();
	if (Operand::isRegister(src1->category()))
		useRegs.push_back(std::static_pointer_cast<Register>(src1));
	if (src2 != nullptr && Operand::isRegister(src2->category()))
		useRegs.push_back(std::static_pointer_cast<Register>(src2));
	if (op == STORE && Operand::isRegister(dst->category())) // dst is also a use-register
		useRegs.push_back(std::static_pointer_cast<Register>(dst));
}

void Branch::renameUseRegs(std::unordered_map<std::shared_ptr<Register>, std::shared_ptr<Register>>& table)
{
	updateRegister(condition, table);
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
	for (auto &arg : args) updateRegister(arg, table);
	if (object != nullptr && Operand::isRegister(object->category()))
		updateRegister(object, table);
}

void Call::updateUseRegs()
{
	useRegs.clear();
	for (auto &arg : args)
		if (Operand::isRegister(arg->category()))
			useRegs.push_back(std::static_pointer_cast<Register>(arg));
	if (object != nullptr && Operand::isRegister(object->category()))
		useRegs.push_back(std::static_pointer_cast<Register>(object));
}

std::shared_ptr<Register> Call::getDefReg()
{
	if (result != nullptr) return std::static_pointer_cast<Register>(result);
	return nullptr;
}

void Call::setDefReg(std::shared_ptr<Register> _defReg)
{
	result = _defReg;
}

void Return::renameUseRegs(std::unordered_map<std::shared_ptr<Register>, std::shared_ptr<Register>>& table)
{
	if (Operand::isRegister(value->category()))
		updateRegister(value, table);
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
	updateRegister(size, table);
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

void updateRegister(std::shared_ptr<Operand>& reg, std::unordered_map<std::shared_ptr<Register>, std::shared_ptr<Register>>& table)
{
	if (!Operand::isRegister(reg->category())) return;
	auto it = table.find(std::static_pointer_cast<Register>(reg));
	if (it != table.end()) reg = it->second;
}
