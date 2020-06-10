#pragma once

#include "pch.h"
#include "Operand.h"
#include "astnode.h"
#include "cfg_visitor.h"

class BasicBlock;
class Function;
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
	using BasicBlockMap = const std::map<std::shared_ptr<BasicBlock>, std::shared_ptr<BasicBlock> > &;
	using OperandMap = const std::map<std::shared_ptr<Operand>, std::shared_ptr<Operand> > &;

	enum InstrTag
	{
		QUADR, 
		CALL, RET, JUMP, BRANCH,   // control-related instructions
		ALLOC,
		PHI,
		// Assembly instructions
		BTYPE, ITYPE, RTYPE,
	};


	IRInstruction(InstrTag _tag, std::weak_ptr<BasicBlock> block) 
		:tag(_tag), residingBlock(block){}
	
	InstrTag getTag() { return tag; }


	std::shared_ptr<BasicBlock> getBlock() { return residingBlock.lock(); }
	void setBlock(std::weak_ptr<BasicBlock> b) { residingBlock = b; }


	std::shared_ptr<IRInstruction> getNextInstr() { return next;}
	void setNextInstr(const std::shared_ptr<IRInstruction> &_next) { next = _next; }

	std::shared_ptr<IRInstruction> getPreviousInstr() { return prev.lock();}
	void setPreviousInstr(const std::shared_ptr<IRInstruction>&_prev) { prev = _prev; }

	// Warning: do no use this directly, use replaceInstruction() instead
	void replaceBy(std::shared_ptr<IRInstruction> i);
	// Warning: do no use this directly, use removeInstruction() instead
	std::vector<std::shared_ptr<Register> > &getUseRegs() { return useRegs; }

	// virtual functions (do nothing by default)
	virtual void renameUseRegs(std::map<std::shared_ptr<Register>,
		std::shared_ptr<Register> > &table) {}
	virtual void updateUseRegs() {}
	virtual std::shared_ptr<Register> getDefReg() { return nullptr; }
	virtual void setDefReg(std::shared_ptr<Register> _defReg) {}

	virtual void accept(CFG_Visitor &vis) = 0;

	virtual void replaceUseReg(std::shared_ptr<Operand> old, std::shared_ptr<Operand> _new) {}

	// for inlining
	virtual std::shared_ptr<IRInstruction>  makeShadow(BasicBlockMap blockMap, OperandMap operandMap) {
		throw Error("do you like that, hah?");
	}
	
protected:
	std::weak_ptr<BasicBlock> residingBlock;
	std::vector<std::shared_ptr<Register> > useRegs;
	std::shared_ptr<IRInstruction> next;
	std::weak_ptr<IRInstruction> prev;  // use weak_ptr to prevent reference cycle
	InstrTag tag;

	// helper functions for inlining
	std::shared_ptr<Operand> getOrDefault(OperandMap mp, std::shared_ptr<Operand> op);
	std::shared_ptr<BasicBlock> getOrDefault(BasicBlockMap mp, std::weak_ptr<BasicBlock> b);
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
		NEG, INV, MOVE,
		// momory access
		LOAD, STORE
	};
	Quadruple(std::weak_ptr<BasicBlock> _block, Operator _op,
		std::shared_ptr<Operand> _dst,
		std::shared_ptr<Operand> _src1, std::shared_ptr<Operand> _src2 = nullptr)  
		:IRInstruction(QUADR,_block), op(_op),dst(_dst),src1(_src1), src2(_src2) {
		updateUseRegs();
	}

	static bool isCompare(Operator op) {
		return op >= LESS && op <= EQ;
	}
	Operator getOp() { return op; }
	std::shared_ptr<Operand> getSrc1() { return src1; }
	std::shared_ptr<Operand> getSrc2() { return src2; }
	std::shared_ptr<Operand> getDst() { return dst; }

	//override functions
	void renameUseRegs(std::map<std::shared_ptr<Register>,
		std::shared_ptr<Register> > &table)override;
	void updateUseRegs()override;
	
	void replaceUseReg(std::shared_ptr<Operand> old, std::shared_ptr<Operand> _new) override;

	std::shared_ptr<Register> getDefReg() override;
	void setDefReg(std::shared_ptr<Register> _defReg) override;

	std::shared_ptr<IRInstruction>  makeShadow(BasicBlockMap blockMap, OperandMap operandMap) override;
;

	ACCEPT_CFG_VISITOR
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
	Branch(std::weak_ptr<BasicBlock> _residingBlock,
		std::shared_ptr<Operand> _cond,
		std::shared_ptr<BasicBlock> _true, std::shared_ptr<BasicBlock> _false)
		:IRInstruction(BRANCH,_residingBlock),
		condition(_cond), trueBlock(_true), falseBlock(_false) {
		updateUseRegs();
	}

	std::shared_ptr<Operand> getCondition() { return condition; }
	std::shared_ptr<BasicBlock> getTrueBlock() { return trueBlock.lock(); }
	std::shared_ptr<BasicBlock> getFalseBlock() { return falseBlock.lock(); }

	void replaceTargetBlock(std::shared_ptr<BasicBlock> old, std::shared_ptr<BasicBlock> b) {
		if (trueBlock.lock() == old) trueBlock = b;
		else if (falseBlock.lock() == old) falseBlock = b;
	}
	//override functions (there is no def-register in branch instructions)
	void renameUseRegs(std::map<std::shared_ptr<Register>,
		std::shared_ptr<Register> > &table)override;
	void updateUseRegs()override;

	void replaceUseReg(std::shared_ptr<Operand> old, std::shared_ptr<Operand> _new) override;

	std::shared_ptr<IRInstruction>  makeShadow(BasicBlockMap blockMap, OperandMap operandMap) override;

	ACCEPT_CFG_VISITOR
private:
	std::shared_ptr<Operand> condition;
	std::weak_ptr<BasicBlock> trueBlock, falseBlock;
};

/*
Class : Call
Constructor: Call(block, funcModule, result = nullptr);
*/
class Call : public IRInstruction {
public:
	Call(std::weak_ptr<BasicBlock> _block,
		std::shared_ptr<Function> _func, std::shared_ptr<Operand> _result = nullptr)
		: IRInstruction(CALL, _block), func(_func), result(_result) {
		updateUseRegs();
	}

	std::shared_ptr<Function> getFunction() { return func; }

	std::vector<std::shared_ptr<Operand> > getArgs() { return args; }
	void addArg(std::shared_ptr<Operand> arg) { args.emplace_back(arg); updateUseRegs(); }
	std::shared_ptr<Operand> getResult() { return result; }
	void setResult(const std::shared_ptr<Operand> &_res) { result = _res; }

	std::shared_ptr<Operand> getObjRef() { return object; }
	void setObjRef(const std::shared_ptr<Operand> &obj) { object = obj; updateUseRegs(); }

	// override functions
	virtual void renameUseRegs(std::map<std::shared_ptr<Register>,
		std::shared_ptr<Register> > &table)override;
	virtual void updateUseRegs()override;

	virtual std::shared_ptr<Register> getDefReg() override;
	virtual void setDefReg(std::shared_ptr<Register> _defReg) override;

	void replaceUseReg(std::shared_ptr<Operand> old, std::shared_ptr<Operand> _new) override;

	std::shared_ptr<IRInstruction>  makeShadow(BasicBlockMap blockMap, OperandMap operandMap) override;

	ACCEPT_CFG_VISITOR
private:
	std::shared_ptr<Function> func;
	std::shared_ptr<Operand> result;
	std::vector<std::shared_ptr<Operand> > args;
	std::shared_ptr<Operand> object;
}; 


class Malloc : public IRInstruction {
public:
	Malloc(std::weak_ptr<BasicBlock> block,
		std::shared_ptr<Operand> _size, std::shared_ptr<Operand> _ptr)
		:IRInstruction(ALLOC, block), size(_size), ptr(_ptr) {
		updateUseRegs();
	}

	std::shared_ptr<Operand> getSize() { return size; }
	void setSize(const std::shared_ptr<Operand> &_size) { size = _size; }

	std::shared_ptr<Operand> getPtr() { return ptr; }
	void setPtr(const std::shared_ptr<Operand> &_ptr) { ptr = _ptr; }
	// override functions
	void renameUseRegs(std::map<std::shared_ptr<Register>,
		std::shared_ptr<Register> > &table)override;
	void updateUseRegs()override;

	std::shared_ptr<Register> getDefReg() override
	{
		return std::static_pointer_cast<Register>(ptr);
	} // is this safe ?
	void setDefReg(std::shared_ptr<Register> _defReg) override { ptr = _defReg; }

	void replaceUseReg(std::shared_ptr<Operand> old, std::shared_ptr<Operand> _new) override;

	std::shared_ptr<IRInstruction>  makeShadow(BasicBlockMap blockMap, OperandMap operandMap) override;

	ACCEPT_CFG_VISITOR
private:
	std::shared_ptr<Operand> size, ptr;
};

class Return : public IRInstruction {
public: 
	Return(std::weak_ptr<BasicBlock> block, std::shared_ptr<Operand> _value)
		:IRInstruction(RET, block), value(_value) {
		updateUseRegs();
	}

	std::shared_ptr<Operand> getValue() { return value; }

	// override functions (no def-register here)
	void renameUseRegs(std::map<std::shared_ptr<Register>,
		std::shared_ptr<Register> > &table)override;
	void updateUseRegs()override;

	void replaceUseReg(std::shared_ptr<Operand> old, std::shared_ptr<Operand> _new) override;

	std::shared_ptr<IRInstruction>  makeShadow(BasicBlockMap blockMap, OperandMap operandMap) override;

	ACCEPT_CFG_VISITOR
private:
	std::shared_ptr<Operand> value;
};

class Jump : public IRInstruction {
public:
	Jump(std::weak_ptr<BasicBlock> _residing, std::weak_ptr<BasicBlock> _target)
		:IRInstruction(JUMP, _residing), target(_target) {}

	std::shared_ptr<BasicBlock> getTarget() { return target.lock(); }
	void setTarget(const std::weak_ptr<BasicBlock> &_target) { target = _target; }
	// virtual functions by default

	std::shared_ptr<IRInstruction>  makeShadow(BasicBlockMap blockMap, OperandMap operandMap) override;

	ACCEPT_CFG_VISITOR
private:
	std::weak_ptr<BasicBlock> target;
};

/*
<PhiFunction> is used in SSA form.
*/
class PhiFunction : public IRInstruction {
public:
	PhiFunction(std::weak_ptr<BasicBlock> _residingBlock, std::shared_ptr<Register> _dst)
		:IRInstruction(PHI, _residingBlock), dst(_dst), origin(_dst){}

	
	void appendRelatedReg(std::shared_ptr<Register> reg, std::shared_ptr<BasicBlock> from);
	std::vector<std::pair<std::shared_ptr<Register>,std::weak_ptr<BasicBlock> > > 
		&getRelatedRegs() { return relatedReg; }
	void removeOption(std::shared_ptr<BasicBlock> b);

	std::shared_ptr<Register> getDst() { return dst; }
	std::shared_ptr<Register> getOrigin() { return origin; }
	//override functions
	virtual std::shared_ptr<Register> getDefReg() override;
	virtual void setDefReg(std::shared_ptr<Register> _defReg) override;

	ACCEPT_CFG_VISITOR
private:
	std::shared_ptr<Register> dst,origin;
	std::vector<std::pair<std::shared_ptr<Register>, std::weak_ptr<BasicBlock> > > relatedReg;
};

// Check if reg is a register and renew it if it is in the table
void updateRegister(std::shared_ptr<Operand> &reg,
	std::map<std::shared_ptr<Register>, std::shared_ptr<Register>>& table);

