#pragma once

#include "pch.h"
#include "Operand.h"
#include "astnode.h"

class BasicBlock;
/*
Class : IRInstruction
The instructions are arranged as a list,
each instruction store the pointer to its
next and previous one.
<residingBlock> indicates to which basic block it belongs.
<regs> are registers used in this instruction.
*/
class IRInstruction {
public:
	enum InstrTag
	{
		QUADR, 
		CALL, RET, JUMP, BRANCH,   // control-related instructions
		ALLOC,
	};
	IRInstruction(InstrTag _tag, std::shared_ptr<BasicBlock> block) 
		:tag(_tag), residingBlock(block){}
	
	std::shared_ptr<BasicBlock> getBlock() { return residingBlock; }

	bool hasNextInstr() { return next != nullptr; }

	std::shared_ptr<IRInstruction> getNextInstr() { return next;}
	void setNextInstr(const std::shared_ptr<IRInstruction> &_next) { next = _next; }

	std::shared_ptr<IRInstruction> getPreviousInstr() { return prev; }
	void setPreviousInstr(const std::shared_ptr<IRInstruction> &_prev) { prev = _prev; }

	std::vector<std::shared_ptr<Register> > &getRegs() { return regs; }

	InstrTag getTag() { return tag; }
protected:
	std::shared_ptr<BasicBlock> residingBlock;
	std::vector<std::shared_ptr<Register> > regs;
	std::shared_ptr<IRInstruction> next, prev;
	InstrTag tag;
};

/*
Class : Quadruple
Constructor : Quadruple(currentBlock, dst, src1, src2 = nullptr);
Note :
This class deal with the binary opeations,
using the same <Operator> enum class as <BinaryExpr>. 
dst <- src1 op src2
src2 could be nullptr, if it is a unary instruction
of the form
dst <- op src1
*/
class Quadruple : public IRInstruction {
public:
	enum Operator
	{
		ADD, MINUS, TIMES, DIVIDE, MOD,
		LESS, LEQ, GREATER, GEQ, NEQ, EQ,
		LSHIFT, RSHIFT,

		BITAND, BITOR, BITXOR,
		//unary
		NEG, INV,
		// momory access
		MOVE, LOAD, STORE
	};
	Quadruple(std::shared_ptr<BasicBlock> _block, Operator _op,
		std::shared_ptr<Operand> _dst,
		std::shared_ptr<Operand> _src1, std::shared_ptr<Operand> _src2 = nullptr)  
		:IRInstruction(QUADR,_block), op(_op),dst(_dst),src1(_src1), src2(_src2) {}

	Operator getOp() { return op; }
	std::shared_ptr<Operand> getSrc1() { return src1; }
	std::shared_ptr<Operand> getSrc2() { return src2; }
	std::shared_ptr<Operand> getDst() { return dst; }
private:
	std::shared_ptr<Operand> dst, src1, src2;
	Operator op;
};

/*
Class: Branch
Constructor: Branch(block, cond, trueBlock, falseBlock)
*/
class Branch : public IRInstruction {
public:
	Branch(std::shared_ptr<BasicBlock> _residingBlock,
		std::shared_ptr<Operand> _cond,
		std::shared_ptr<BasicBlock> _true, std::shared_ptr<BasicBlock> _false)
		:IRInstruction(BRANCH,_residingBlock),
		condition(_cond), trueBlock(_true), falseBlock(_false) {}

	std::shared_ptr<Operand> getCondition() { return condition; }
	std::shared_ptr<BasicBlock> getTrueBlock() { return trueBlock; }
	std::shared_ptr<BasicBlock> getFalseBlock() { return falseBlock; }
private:
	std::shared_ptr<Operand> condition;
	std::shared_ptr<BasicBlock> trueBlock, falseBlock;
};

class Call : public IRInstruction {
public:
	Call(std::shared_ptr<BasicBlock> _block,
		std::shared_ptr<Function> _func, std::shared_ptr<Operand> _result)
		: IRInstruction(CALL, _block), func(_func), result(_result) {}

	std::shared_ptr<Function> getFunction() { return func; }

	std::vector<std::shared_ptr<Operand> > getArgs() { return args; }
	void addArg(std::shared_ptr<Operand> arg) { args.emplace_back(arg); }
	std::shared_ptr<Operand> getResult() { return result; }
	void setResult(const std::shared_ptr<Operand> &_res) { result = _res; }

	std::shared_ptr<Operand> getInstance() { return instancePtr;}
	void setInstance(const std::shared_ptr<Operand> &_instance) { instancePtr = _instance; }
private:
	std::shared_ptr<Function> func;
	std::shared_ptr<Operand> result, instancePtr;
	std::vector<std::shared_ptr<Operand> > args;
}; 



class Malloc : public IRInstruction {
public:
	Malloc(std::shared_ptr<BasicBlock> block,
		std::shared_ptr<Operand> _size, std::shared_ptr<Operand> _ptr)
		:IRInstruction(ALLOC, block), size(_size), ptr(_ptr) {}

	std::shared_ptr<Operand> getSize() { return size; }
	void setSize(const std::shared_ptr<Operand> &_size) { size = _size; }

	std::shared_ptr<Operand> getPtr() { return ptr; }
	void setPtr(const std::shared_ptr<Operand> &_ptr) { ptr = _ptr; }
private:
	std::shared_ptr<Operand> size, ptr;
};

class Return : public IRInstruction {
public: 
	Return(std::shared_ptr<BasicBlock> block, std::shared_ptr<Operand> _value)
		:IRInstruction(RET, block), value(_value) {}

	std::shared_ptr<Operand> getValue() { return value; }
private:
	std::shared_ptr<Operand> value;
};

class Jump : public IRInstruction {
public:
	Jump(std::shared_ptr<BasicBlock> _residing, std::shared_ptr<BasicBlock> _target)
		:IRInstruction(JUMP, _residing), target(_target) {}

	std::shared_ptr<BasicBlock> getTarget() { return target; }
	void setTarget(const std::shared_ptr<BasicBlock> &_target) { target = _target; }
	std::shared_ptr<BasicBlock> getTarget() { return target; }
private:
	std::shared_ptr<BasicBlock> target;
};
