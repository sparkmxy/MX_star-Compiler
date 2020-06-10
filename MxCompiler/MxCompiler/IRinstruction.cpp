#include "IRinstruction.h"

void Quadruple::renameUseRegs(std::map<std::shared_ptr<Register>, std::shared_ptr<Register>>& table)
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

void Quadruple::replaceUseReg(std::shared_ptr<Operand> old, std::shared_ptr<Operand> _new)
{
	if (src1 == old) src1 = _new;
	if (src2 == old) src2 = _new;
	if (op == STORE && dst == old) dst = _new;
	updateUseRegs();
}

std::shared_ptr<Register> Quadruple::getDefReg()
{
	if (op == STORE) return nullptr;
	return std::static_pointer_cast<Register>(dst);
}

void Quadruple::setDefReg(std::shared_ptr<Register> _defReg)
{
	if (op != STORE) dst = _defReg;
}

std::shared_ptr<IRInstruction> Quadruple::makeShadow(BasicBlockMap blockMap, OperandMap operandMap)
{
	return std::make_shared<Quadruple>(getOrDefault(blockMap, residingBlock), op,
		getOrDefault(operandMap, dst), getOrDefault(operandMap, src1), getOrDefault(operandMap, src2));
}

void Branch::renameUseRegs(std::map<std::shared_ptr<Register>, std::shared_ptr<Register>>& table)
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

void Branch::replaceUseReg(std::shared_ptr<Operand> old, std::shared_ptr<Operand> _new)
{
	if(condition == old) condition = _new;
	updateUseRegs();
}

std::shared_ptr<IRInstruction> Branch::makeShadow(BasicBlockMap blockMap, OperandMap operandMap)
{
	return std::make_shared<Branch>(getOrDefault(blockMap, residingBlock), getOrDefault(operandMap, condition),
		getOrDefault(blockMap, trueBlock), getOrDefault(blockMap, falseBlock));
}

void Call::renameUseRegs(std::map<std::shared_ptr<Register>, std::shared_ptr<Register>>& table)
{
	for (int i = 0; i < args.size(); i++) updateRegister(args[i], table);
	if (object != nullptr && Operand::isRegister(object->category()))
		updateRegister(object, table);
	updateUseRegs();
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
	if (result != nullptr) 
		return std::static_pointer_cast<Register>(result);
	return nullptr;
}

void Call::setDefReg(std::shared_ptr<Register> _defReg)
{
	result = _defReg;
}

void Call::replaceUseReg(std::shared_ptr<Operand> old, std::shared_ptr<Operand> _new)
{
	for (int i = 0; i < args.size(); i++)
		if (args[i] == old) args[i] = _new;
	if (object == old) object = _new;
	updateUseRegs();
}

std::shared_ptr<IRInstruction> Call::makeShadow(BasicBlockMap blockMap, OperandMap operandMap)
{
	auto shadow = std::make_shared<Call>(getOrDefault(blockMap, residingBlock), func, getOrDefault(operandMap, result));
	shadow->object = (getOrDefault(operandMap, object));
	for (auto arg : args) shadow->args.push_back(getOrDefault(operandMap,arg));
	shadow->updateUseRegs();

	return shadow;
}

void Return::renameUseRegs(std::map<std::shared_ptr<Register>, std::shared_ptr<Register>>& table)
{
	if (value != nullptr && Operand::isRegister(value->category()))
		updateRegister(value, table);
	updateUseRegs();
}

void Return::updateUseRegs()
{
	useRegs.clear();
	if (value != nullptr && Operand::isRegister(value->category())) 
		useRegs.push_back(std::static_pointer_cast<Register>(value));
}

void Return::replaceUseReg(std::shared_ptr<Operand> old, std::shared_ptr<Operand> _new)
{
	if (value == old) value = _new;
	updateUseRegs();
}

std::shared_ptr<IRInstruction> Return::makeShadow(BasicBlockMap blockMap, OperandMap operandMap)
{
	return std::make_shared<Return>(getOrDefault(blockMap, residingBlock), getOrDefault(operandMap, value));
}

void Malloc::renameUseRegs(std::map<std::shared_ptr<Register>, std::shared_ptr<Register>>& table)
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

void Malloc::replaceUseReg(std::shared_ptr<Operand> old, std::shared_ptr<Operand> _new)
{
	if(size == old) size = _new;
	updateUseRegs();
}

std::shared_ptr<IRInstruction> Malloc::makeShadow(BasicBlockMap blockMap, OperandMap operandMap)
{
	return std::make_shared<Malloc>(getOrDefault(blockMap, residingBlock),
		getOrDefault(operandMap, size), getOrDefault(operandMap, ptr));
}


void PhiFunction::appendRelatedReg(std::shared_ptr<Register> reg, std::shared_ptr<BasicBlock> from)
{
	relatedReg.push_back(std::make_pair(reg,from));
}

void PhiFunction::removeOption(std::shared_ptr<BasicBlock> b)
{
	for(auto it = relatedReg.begin(); it != relatedReg.end(); it++)
		if (it->second.lock() == b) {
			relatedReg.erase(it);
			break;
		}
}

std::shared_ptr<Register> PhiFunction::getDefReg()
{
	return dst;
}

void PhiFunction::setDefReg(std::shared_ptr<Register> _defReg)
{
	dst = _defReg;
}

void updateRegister(std::shared_ptr<Operand>& reg, std::map<std::shared_ptr<Register>, std::shared_ptr<Register>>& table)
{
	if (!Operand::isRegister(reg->category())) return;
	auto it = table.find(std::static_pointer_cast<Register>(reg));
	if (it != table.end()) reg = it->second;
}

void IRInstruction::replaceBy(std::shared_ptr<IRInstruction> i)
{
	i->prev = prev;
	i->next = next;
	if (next != nullptr) next->prev = i;
	if(prev.lock() != nullptr) prev.lock()->next = i;
}

std::shared_ptr<Operand> IRInstruction::getOrDefault(OperandMap mp, std::shared_ptr<Operand> op)
{
	if (op == nullptr) return nullptr;
	auto it = mp.find(op);
	return it == mp.end() ? op : it->second;
}

std::shared_ptr<BasicBlock> IRInstruction::getOrDefault(BasicBlockMap mp, std::weak_ptr<BasicBlock> b)
{
	auto it = mp.find(b.lock());
	return it == mp.end() ? b.lock() : it->second;
}

std::shared_ptr<IRInstruction> Jump::makeShadow(BasicBlockMap blockMap, OperandMap operandMap)
{
	return std::make_shared<Jump>(getOrDefault(blockMap, residingBlock), getOrDefault(blockMap, target));
}
