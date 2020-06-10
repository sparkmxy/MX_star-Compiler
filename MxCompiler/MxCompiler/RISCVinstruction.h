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
	RISCVinstruction(std::weak_ptr<RISCVBasicBlock> b, Category _c);
		

	std::shared_ptr<RISCVBasicBlock> getBlock() { return residingBlk.lock(); }

	std::shared_ptr<RISCVinstruction> getNextInstr() { return next; }
	void setNextInstr(const std::shared_ptr<RISCVinstruction> & i) { next = i; }

	std::shared_ptr<RISCVinstruction> getPrevInstr() { return prev.lock(); }
	void setPrevInstr(const std::shared_ptr<RISCVinstruction> & i) { prev = i; }

	Category category() { return c; }

	// virtual functions
	virtual std::string toString() = 0;
	virtual std::vector<std::shared_ptr<Register> > getUseReg() { return {}; }
	virtual std::shared_ptr<Register> getDefReg() { return nullptr; }
	
	virtual void updateUseReg(std::shared_ptr<Register> reg, std::shared_ptr<Register> new_reg){}
	virtual void updateDefReg(std::shared_ptr<Register> new_reg){}

private:
	Category c;
	std::weak_ptr<RISCVBasicBlock> residingBlk;
	std::shared_ptr<RISCVinstruction> next;
	std::weak_ptr<RISCVinstruction> prev;
};


class B_type : public RISCVinstruction {
public:
	enum CmpOp
	{
		BEQ, BNE, BLE, BGE, BLT, BGT
	};
	static const std::string op_to_string[];

	B_type(std::weak_ptr<RISCVBasicBlock> b, CmpOp _op,
		std::shared_ptr<Register> _rs1, std::shared_ptr<Register> _rs2,
		std::shared_ptr<RISCVBasicBlock> _target)
		:RISCVinstruction(b, BTYPE), rs1(_rs1), rs2(_rs2), target(_target), op(_op){}

	std::shared_ptr<Register> getRs1() { return rs1; }
	std::shared_ptr<Register> getRs2() { return rs2; }

	std::shared_ptr<RISCVBasicBlock> getTargetBlock() { return target; }

	// virtual functions
	std::string toString() override;
	std::vector<std::shared_ptr<Register> > getUseReg() override;

	void updateUseReg(std::shared_ptr<Register> reg, std::shared_ptr<Register> new_reg) override;

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

	static const std::string op_to_string[];

	I_type(std::weak_ptr<RISCVBasicBlock> b, Operator _op,
		std::shared_ptr<Register> _rd, std::shared_ptr<Register> _rs1, std::shared_ptr<Immediate> _imm)
		:RISCVinstruction(b, ITYPE), rd(_rd), rs1(_rs1), op(_op), imm(_imm) {}

	std::shared_ptr<Register> getRd() { return rd; }
	std::shared_ptr<Register> getRs1() { return rs1; }
	std::shared_ptr<Immediate> getImm() { return imm; }

	std::string toString() override;
	std::vector<std::shared_ptr<Register> > getUseReg() override;
	std::shared_ptr<Register> getDefReg() override;

	void updateUseReg(std::shared_ptr<Register> reg, std::shared_ptr<Register> new_reg)override;
	void updateDefReg(std::shared_ptr<Register> new_reg)override;
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

	static const std::string op_to_string[];

	R_type(std::weak_ptr<RISCVBasicBlock> b, Operator _op,
		std::shared_ptr<Register> _rd, std::shared_ptr<Register> _rs1, std::shared_ptr<Register> _rs2)
		:RISCVinstruction(b, RTYPE), op(_op), rd(_rd), rs1(_rs1), rs2(_rs2) {}

	std::shared_ptr<Register> getRd() { return rd; }
	
	std::shared_ptr<Register>getRS1() { return rs1; }
	std::shared_ptr<Register> getRs2() { return rs2; }

	std::string toString() override;
	std::vector<std::shared_ptr<Register> > getUseReg() override;
	std::shared_ptr<Register> getDefReg() override;

	void updateUseReg(std::shared_ptr<Register> reg, std::shared_ptr<Register> new_reg)override;
	void updateDefReg(std::shared_ptr<Register> new_reg)override;

private:
	std::shared_ptr<Register> rs1, rs2, rd;
	Operator op;
};

class CallAssembly : public RISCVinstruction {
public:
	CallAssembly(std::weak_ptr<RISCVBasicBlock> b, std::shared_ptr<RISCVFunction> f)
		:RISCVinstruction(b, CALL), func(f) {}

	std::shared_ptr<RISCVFunction> getFunction() { return func; }
	std::string toString() override;
private:
	std::shared_ptr<RISCVFunction> func;
};

class MoveAssembly : public RISCVinstruction {
public:
	MoveAssembly(std::weak_ptr<RISCVBasicBlock> b,
		std::shared_ptr<Register> _rd, std::shared_ptr<Register> _rs1)
		:RISCVinstruction(b, MOV), rd(_rd), rs1(_rs1) {}

	std::shared_ptr<Register> getRd() { return rd; }

	std::shared_ptr<Register> getRs1() { return rs1; }

	std::string toString() override;
	std::vector<std::shared_ptr<Register> > getUseReg() override;
	std::shared_ptr<Register> getDefReg() override;

	void updateUseReg(std::shared_ptr<Register> reg, std::shared_ptr<Register> new_reg)override;
	void updateDefReg(std::shared_ptr<Register> new_reg)override;
private:
	std::shared_ptr<Register> rs1, rd;
};

class RetAssembly : public RISCVinstruction {
public:
	RetAssembly(std::weak_ptr<RISCVBasicBlock> b) :RISCVinstruction(b, RET) {}
	std::string toString() override { return "ret"; }
};

class JumpAssembly :public RISCVinstruction {
public:
	JumpAssembly(std::weak_ptr<RISCVBasicBlock> b, std::shared_ptr<RISCVBasicBlock> _target)
		:RISCVinstruction(b, JUMP), target(_target) {}

	std::shared_ptr<RISCVBasicBlock> getTarget() { return target; }
	std::string toString() override;

private:
	std::shared_ptr<RISCVBasicBlock> target;
};

class LoadImm : public RISCVinstruction {
public:
	LoadImm(std::weak_ptr<RISCVBasicBlock> b,
		std::shared_ptr<Register> _rd, std::shared_ptr<Immediate> i)
		:RISCVinstruction(b, LI), rd(_rd), imm(i) {}

	std::shared_ptr<Register> getRd() { return rd; }
	std::shared_ptr<Immediate> getImm() { return imm; }

	std::string toString() override;
	std::shared_ptr<Register> getDefReg() override;

	void updateDefReg(std::shared_ptr<Register> new_reg)override;
private:
	std::shared_ptr<Register> rd;
	std::shared_ptr<Immediate> imm;
};

class LoadAddr :public RISCVinstruction {
public:
	LoadAddr(std::weak_ptr<RISCVBasicBlock> b,
		std::shared_ptr<Register> _rd, std::shared_ptr<StaticString> s)
		:RISCVinstruction(b, LA), rd(_rd), symbol(s) {}

	std::shared_ptr<Register> getRd() { return rd; }

	std::shared_ptr<StaticString> getSymbol() { return symbol; }

	std::string toString() override;
	std::shared_ptr<Register> getDefReg() override;

	void updateDefReg(std::shared_ptr<Register> new_reg)override;
private:
	std::shared_ptr<Register> rd;
	std::shared_ptr<StaticString> symbol;
};

class Load : public RISCVinstruction {
public:
	Load(std::weak_ptr<RISCVBasicBlock> b,
		std::shared_ptr<Address> _addr, std::shared_ptr<Register> _rd, int _size)
		:RISCVinstruction(b, LOAD), rd(_rd), size(_size), addr(_addr){}

	std::shared_ptr<Register> getRd() { return rd; }
	std::shared_ptr<Address> getAddr() { return addr; }

	std::string toString() override;
	std::vector<std::shared_ptr<Register> > getUseReg() override;  
	std::shared_ptr<Register> getDefReg() override;

	void updateUseReg(std::shared_ptr<Register> reg, std::shared_ptr<Register> new_reg)override;
	void updateDefReg(std::shared_ptr<Register> new_reg)override;
private:
	std::shared_ptr<Register> rd;
	std::shared_ptr<Address> addr; // This can be a register or a stackLocation
	int size;
};

// Constructor: residingBlock, addr, rs, size
class Store : public RISCVinstruction {
public:
	Store(std::weak_ptr<RISCVBasicBlock> b,
		std::shared_ptr<Address> _addr, std::shared_ptr<Register> _rs, int _size, std::shared_ptr<Register> _rt = nullptr)
		:RISCVinstruction(b, STORE), addr(_addr), rs(_rs), size(_size), rt(_rt){}

	std::shared_ptr<Register> getRs() { return rs; }
	std::shared_ptr<Register> getRt() { return rt; }
	std::shared_ptr<Address> getAddr() { return addr; }
	int getSize() { return size; }

	std::string toString() override;
	std::vector<std::shared_ptr<Register> > getUseReg() override;
	std::shared_ptr<Register> getDefReg() override;

	void updateUseReg(std::shared_ptr<Register> reg, std::shared_ptr<Register> new_reg)override;
	void updateDefReg(std::shared_ptr<Register> new_reg)override;
private:
	std::shared_ptr<Register> rs, rt;
	std::shared_ptr<Address> addr;  // This can be a register or a stackLocation
	int size;
}; 