#pragma once

#include "pch.h"
#include "Operand.h"
#include "RISCVOperand.h"


class RISCVBasicBlock;
class RISCVFunction;

class RISCVinstruction {
public:
	enum Category
	{
		BTYPE, ITYPE, RTYPE,
		CALL, MOV, RET, JUMP,
		LOAD, STORE,
		LI, LA
	};
	RISCVinstruction(std::shared_ptr<RISCVBasicBlock> b, Category _c)
		:residingBlk(b), c(_c) {}

	std::shared_ptr<RISCVBasicBlock> getBlock() { return residingBlk; }

	std::shared_ptr<RISCVinstruction> getNextInstr() { return next; }
	void setNextInstr(const std::shared_ptr<RISCVinstruction> & i) { next = i; }

	std::shared_ptr<RISCVinstruction> getPrevInstr() { return prev.lock(); }
	void setPrevInstr(const std::shared_ptr<RISCVinstruction> & i) { prev = i; }
	
	void removeThis();

	Category category() { return c; }

	// virtual functions
	virtual std::string toString() = 0;

private:
	Category c;
	std::shared_ptr<RISCVBasicBlock> residingBlk;
	std::shared_ptr<RISCVinstruction> next;
	std::weak_ptr<RISCVinstruction> prev;
};


class B_type : public RISCVinstruction {
public:
	enum CmpOp
	{
		BEQ, BNE, BLE, BGE, BLT, BGT
	};

	B_type(std::shared_ptr<RISCVBasicBlock> b, CmpOp _op,
		std::shared_ptr<Register> _rs1, std::shared_ptr<Register> _rs2,
		std::shared_ptr<RISCVBasicBlock> _target)
		:RISCVinstruction(b, BTYPE), rs1(_rs1), rs2(_rs2), target(_target), op(_op){}

	std::shared_ptr<Register> getRs1() { return rs1; }
	std::shared_ptr<Register> getRs2() { return rs2; }

	std::shared_ptr<RISCVBasicBlock> getTargetBlock() { return target; }
private:

	std::shared_ptr<Register> rs1, rs2; 
	std::shared_ptr<RISCVBasicBlock> target;
	CmpOp op;
};


class I_type : public RISCVinstruction {
public:
	enum Operator
	{
		ADDI, XORI, ORI, ANDI, 
		SLTI, SLTIU, SLLI, SRAI
	};

	I_type(std::shared_ptr<RISCVBasicBlock> b, Operator _op,
		std::shared_ptr<Register> _rd, std::shared_ptr<Register> _rs1, std::shared_ptr<Immediate> _imm)
		:RISCVinstruction(b, ITYPE), rd(_rd), rs1(_rs1), op(_op), imm(_imm) {}

	std::shared_ptr<Register> getRd() { return rd; }
	std::shared_ptr<Register> getRs1() { return rs1; }
	std::shared_ptr<Immediate> getImm() { return imm; }
private:
	std::shared_ptr<Register> rd, rs1;
	std::shared_ptr<Immediate> imm;
	Operator op;
};

class R_type : public RISCVinstruction {
public:
	enum Operator
	{
		ADD, SUB, MUL, DIV, MOD, 
		SLL, SRA, SLT, SLTU,
		AND, OR, XOR
	};

	R_type(std::shared_ptr<RISCVBasicBlock> b, Operator _op,
		std::shared_ptr<Register> _rd, std::shared_ptr<Register> _rs1, std::shared_ptr<Register> _rs2)
		:RISCVinstruction(b, RTYPE), op(_op), rd(_rd), rs1(_rs1), rs2(_rs2) {}

	std::shared_ptr<Register> getRd() { return rd; }
	
	std::shared_ptr<Register>getRS1() { return rs1; }
	std::shared_ptr<Register> getRs2() { return rs2; }
private:
	std::shared_ptr<Register> rs1, rs2, rd;
	Operator op;
};

class CallAssembly : public RISCVinstruction {
public:
	CallAssembly(std::shared_ptr<RISCVBasicBlock> b, std::shared_ptr<RISCVFunction> f)
		:RISCVinstruction(b, CALL), func(f) {}

	std::shared_ptr<RISCVFunction> getFunction() { return func; }
private:
	std::shared_ptr<RISCVFunction> func;
};

class MoveAssembly : public RISCVinstruction {
public:
	MoveAssembly(std::shared_ptr<RISCVBasicBlock> b,
		std::shared_ptr<Register> _rd, std::shared_ptr<Register> _rs1)
		:RISCVinstruction(b, MOV), rd(_rd), rs1(_rs1) {}

	std::shared_ptr<Register> getRd() { return rd; }

	std::shared_ptr<Register> getRs1() { return rs1; }
private:
	std::shared_ptr<Register> rs1, rd;
};

class RetAssembly : public RISCVinstruction {
public:
	RetAssembly(std::shared_ptr<RISCVBasicBlock> b) :RISCVinstruction(b, RET) {}
};

class JumpAssembly :public RISCVinstruction {
public:
	JumpAssembly(std::shared_ptr<RISCVBasicBlock> b, std::shared_ptr<RISCVBasicBlock> _target)
		:RISCVinstruction(b, JUMP), target(_target) {}

	std::shared_ptr<RISCVBasicBlock> getTarget() { return target; }
private:
	std::shared_ptr<RISCVBasicBlock> target;
};

class LoadImm : public RISCVinstruction {
public:
	LoadImm(std::shared_ptr<RISCVBasicBlock> b,
		std::shared_ptr<Register> _rd, std::shared_ptr<Immediate> i)
		:RISCVinstruction(b, LI), rd(_rd), imm(i) {}

	std::shared_ptr<Register> getRd() { return rd; }
	std::shared_ptr<Immediate> getImm() { return imm; }
private:
	std::shared_ptr<Register> rd;
	std::shared_ptr<Immediate> imm;
};

class LoadAddr :public RISCVinstruction {
public:
	LoadAddr(std::shared_ptr<RISCVBasicBlock> b,
		std::shared_ptr<Register> _rd, std::shared_ptr<StaticString> s)
		:RISCVinstruction(b, LA), rd(_rd), symbol(s) {}

	std::shared_ptr<Register> getRd() { return rd; }

	std::shared_ptr<StaticString> getSymbol() { return symbol; }
private:
	std::shared_ptr<Register> rd;
	std::shared_ptr<StaticString> symbol;
};

// TO DO : Load and Store