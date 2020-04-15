#include "ir_printer.h"


void IR_Printer::print()
{
	os << "IR code: \n";
	visit(ir.get());
	os << "------------------------------------------------------------\n";
}

void IR_Printer::visit(IR * ir)
{
	auto glbVars = ir->getGlbVars();
	for (auto &var : glbVars)
		os << "@" << var->getName() << '\n';
	// Shall we deal with static strings ? 
	auto functions = ir->getFunctions();
	for (auto &f : functions) f->accept(*this);
}

void IR_Printer::visit(Function * f)
{
	auto retType = f->isVoid() ?  "void" : "i32";
	os << "def " << retType << " @" << f->getName() << "{ \n";
	auto blocks = f->getBlockList();
	for (auto &block : blocks) block->accept(*this);
	os << "}\n";
}

void IR_Printer::visit(BasicBlock * b)
{
	os << getLabel(b) << ":\n";
	for (auto instr = b->getFront(); instr != nullptr; instr = instr->getNextInstr()) {
		os << "    ";
		instr->accept(*this);
		os << '\n';
	}
}

void IR_Printer::visit(Quadruple * q)
{
	std::string op;
	auto op_tag = q->getOp();
	switch (op_tag)   // op = op_tag.toString()
	{
	case Quadruple::ADD:    op = "add";
		break;
	case Quadruple::MINUS:  op = "sub";
		break;
	case Quadruple::TIMES:  op = "mul";
		break;
	case Quadruple::DIVIDE: op = "div";
		break;
	case Quadruple::MOD:    op = "mod";
		break;
	case Quadruple::LESS:   op = "slt";
		break;
	case Quadruple::LEQ:    op = "sle";
		break;
	case Quadruple::GREATER: op = "sgt";
		break;
	case Quadruple::GEQ:     op = "sge";
		break;
	case Quadruple::NEQ:     op = "sne";
		break;
	case Quadruple::EQ:      op = "seq";
		break;
	case Quadruple::LSHIFT:  op = "shl";
		break;
	case Quadruple::RSHIFT:  op = "shr";
		break;
	case Quadruple::BITAND:  op = "and";
		break;
	case Quadruple::BITOR:   op = "or";
		break;
	case Quadruple::BITXOR:  op = "xor";
		break;
	case Quadruple::NEG:     op = "neg";
		break;
	case Quadruple::INV:     op = "inv";
		break;
	case Quadruple::LOAD:    op = "load";
		break;
	case Quadruple::STORE:   op = "store";
		break;
	case Quadruple::MOVE:    op = "mov";
		break;
	default:
		break;
	}   

	os << op << " ";
	q->getDst()->accept(*this);
	os << " ";
	q->getSrc1()->accept(*this);
	auto src2 = q->getSrc2();
	if (src2 != nullptr) {
		os << " ";
		src2->accept(*this);
	}
}

void IR_Printer::visit(Branch * b)
{
	os << "br ";
	b->getCondition()->accept(*this);
	os << " " << getLabel(b->getTrueBlock().get()) << " " << getLabel(b->getFalseBlock().get());
}

void IR_Printer::visit(Call * c)
{
	if (c->getResult() != nullptr) {   // call with a return value
		c->getResult()->accept(*this);
		os << " = call ";
	}
	else os << "call ";

	os << c->getFunction()->getName() << " ";
	if (c->getObjRef() != nullptr) {
		c->getObjRef()->accept(*this);
		os << " ";
	}
	auto args = c->getArgs();
	for (auto &arg : args) {
		arg->accept(*this);
		os << " ";
	}
}

void IR_Printer::visit(Malloc * m)
{
	m->getPtr()->accept(*this);
	os << " = malloc ";
	m->getSize()->accept(*this);
}

void IR_Printer::visit(Return * r)
{
	os << "ret ";
	if (r->getValue() != nullptr) r->getValue()->accept(*this);
}

void IR_Printer::visit(Jump * j)
{
	os << "jmp " << getLabel(j->getTarget().get());
}

void IR_Printer::visit(PhiFunction * p)
{
	// do nothing for now
}

void IR_Printer::visit(Register * r)
{
	// should we make global vars different?
	os << "%" << getName(r);
}

void IR_Printer::visit(StaticString * s)
{
	os << "%" << getName(s->getReg().get());
}

void IR_Printer::visit(Immediate * i)
{
	os << '#' << i->getValue();
}

std::string IR_Printer::getName(Register * reg)
{
	if (nameForReg.find(reg) == nameForReg.end()) return newRegName(reg);
	return nameForReg[reg];
}

std::string IR_Printer::newRegName(Register *reg)
{
	std::string newNameBase = reg->getName() == "" ? "t" : reg->getName();   
	// it could be a temperory register with no name, we name it "$t"
	auto newName = newNameBase + std::to_string(nameCounter[newNameBase]);
	nameCounter[newNameBase]++;
	nameForReg[reg] = newName;
	return newName;
}

std::string IR_Printer::getLabel(BasicBlock * block)
{
	if (block == nullptr) return ""; //is this ok?
	if (label.find(block) == label.end()) return newLabel(block);
	return label[block];
}

std::string IR_Printer::newLabel(BasicBlock * block)
{
	std::string labelBase = block->toString();
	auto new_label = labelBase + std::to_string(nameCounter[labelBase]);
	nameCounter[labelBase]++;
	return new_label;
}







