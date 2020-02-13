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
	IRInstruction(std::shared_ptr<BasicBlock> block) : residingBlock(block){}
	
	std::shared_ptr<BasicBlock> getBlock() { return residingBlock; }

	bool hasNextInstr() { return next != nullptr; }

	std::shared_ptr<IRInstruction> getNextInstr() { return next;}
	void setNextInstr(const std::shared_ptr<IRInstruction> &_next) { next = _next; }

	std::shared_ptr<IRInstruction> getPreviousInstr() { return prev; }
	void setPreviousInstr(const std::shared_ptr<IRInstruction> &_prev) { prev = _prev; }

	std::vector<std::shared_ptr<Register> > &getRegs() { return regs; }
protected:
	std::shared_ptr<BasicBlock> residingBlock;
	std::vector<std::shared_ptr<Register> > regs;
	std::shared_ptr<IRInstruction> next, prev;
};

/*
Class : Quadruple
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
		
		AND, NOT, OR,
		BITAND, BITOR, BITXOR,
		//Unary
		NEG, POS, NOT, INV,
		PREINC, POSTINC,
		PREDEC, POSTDEC, ASSIGN // is ASSIGN special ?
	};
	Quadruple(std::shared_ptr<BasicBlock> _block, Operator _op,
		std::shared_ptr<Operand> _dst,
		std::shared_ptr<Operand> _src1, std::shared_ptr<Operand> _src2)
		:IRInstruction(_block), op(_op),dst(_dst),src1(_src1), src2(_src2) {}

	Operator getOp() { return op; }
	std::shared_ptr<Operand> getSrc1() { return src1; }
	std::shared_ptr<Operand> getSrc2() { return src2; }
	std::shared_ptr<Operand> getDst() { return dst; }
private:
	std::shared_ptr<Operand> dst, src1, src2;
	Operator op;
};

class Branch : public IRInstruction {
public:
	Branch(std::shared_ptr<Operand> _cond,
		std::shared_ptr<BasicBlock> _residingBlock,
		std::shared_ptr<BasicBlock> _then, std::shared_ptr<BasicBlock> _else)
		:IRInstruction(_residingBlock),
		condition(_cond), thenBlock(_then), elseBlock(_else) {}

	std::shared_ptr<Operand> getCondition() { return condition; }
	std::shared_ptr<BasicBlock> getThenBlock() { return thenBlock; }
	std::shared_ptr<BasicBlock> getElseBlock() { return elseBlock; }
private:
	std::shared_ptr<Operand> condition;
	std::shared_ptr<BasicBlock> thenBlock, elseBlock;
};

class Call : public IRInstruction {
public:
	Call(std::shared_ptr<BasicBlock> _block,
		std::shared_ptr<Function> _func, std::shared_ptr<Operand> _result)
		: IRInstruction(_block), func(_func), result(_result) {}

	std::shared_ptr<Function> getFunction() { return func; }
	std::vector<std::shared_ptr<Operand> > getParameters() { return parameters; }
	std::shared_ptr<Operand> getResult() { return result; }

	std::shared_ptr<Operand> getInstance() { return instancePtr;}
	void setInstance(const std::shared_ptr<Operand> &_instance) { instancePtr = _instance; }
private:
	std::shared_ptr<Function> func;
	std::shared_ptr<Operand> result, instancePtr;
	std::vector<std::shared_ptr<Operand> > parameters;
}; 

/*
Class : Load
Load the value at the address <src> to <dst>.
*/
class Load : IRInstruction {
public:
	Load(std::shared_ptr<BasicBlock> block,
		std::shared_ptr<Operand> _src, std::shared_ptr<Operand> _dst)
		: IRInstruction(block), src(_src), dst(_dst) {}

	std::shared_ptr<Operand> getSrc() { return src; }
	std::shared_ptr<Operand> getDst() { return dst; }
private:
	std::shared_ptr<Operand> src, dst;
};

/*
Class : Store
Store the value in <src> to the address <dst>.
*/
class Store : IRInstruction {
public:
	Store(std::shared_ptr<BasicBlock> block,
		std::shared_ptr<Operand> _src, std::shared_ptr<Operand> _dst)
		: IRInstruction(block), src(_src), dst(_dst) {}

	std::shared_ptr<Operand> getSrc() { return src; }
	std::shared_ptr<Operand> getDst() { return dst; }
private:
	std::shared_ptr<Operand> src, dst;
};

class Malloc : IRInstruction {
public:
	Malloc(std::shared_ptr<BasicBlock> block,
		std::shared_ptr<Operand> _size, std::shared_ptr<Operand> _ptr)
		:IRInstruction(block), size(_size), ptr(_ptr) {}

	std::shared_ptr<Operand> getSize() { return size; }
	void setSize(const std::shared_ptr<Operand> &_size) { size = _size; }

	std::shared_ptr<Operand> getPtr() { return ptr; }
	void setPtr(const std::shared_ptr<Operand> &_ptr) { ptr = _ptr; }
private:
	std::shared_ptr<Operand> size, ptr;
};

class Return : IRInstruction {
public: 
	Return(std::shared_ptr<BasicBlock> block, std::shared_ptr<Operand> _value)
		:IRInstruction(block), value(_value) {}

	std::shared_ptr<Operand> getValue() { return value; }
private:
	std::shared_ptr<Operand> value;
};

class Jump : IRInstruction {
public:
	Jump(std::shared_ptr<BasicBlock> _residing, std::shared_ptr<BasicBlock> _target)
		:IRInstruction(_residing), target(_target) {}

	std::shared_ptr<BasicBlock> getTarget() { return target; }
	void setTarget(const std::shared_ptr<BasicBlock> &_target) { target = _target; }
private:
	std::shared_ptr<BasicBlock> target;
};