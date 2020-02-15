#include "IR_Generator.h"

void IR_Generator::visit(ProgramAST * node)
{
	auto decls = node->getDecls();
	scanGlobalVar = true;
	for (auto &decl : decls)
		if(decl->isVarDecl())decl->accept(*this);
	scanGlobalVar = false;
	for (auto &decl : decls)
		if (!decl->isVarDecl()) decl->accept(*this);
}

void IR_Generator::visit(MultiVarDecl * node)
{
	auto vars = node->getDecls();
	for (auto &var : vars) var->accept(*this);
}

void IR_Generator::visit(VarDeclStmt * node)
{
	auto varSymbol = node->getVarSymbol();
	auto reg = std::make_shared<VirtualReg>(node->getIdentifier()->name);
	varSymbol->setReg(reg);
	if (scanGlobalVar)  // global variable
		ir->addGlobalVar(reg);
	else {
		if (currentFunction != nullptr && node->isArgument())
			currentFunction->appendArg(reg);
		auto initExpr = node->getInitExpr();
		if (initExpr != nullptr) {
			initExpr->accept(*this);
			assign(reg, initExpr.get());
		}
	}
}

void IR_Generator::visit(FunctionDecl * node)
{
	auto symbol = node->getFuncSymbol();
	auto funcModule = std::make_shared<Function>(symbol->getSymbolName());
	symbol->setModule(funcModule);
	currentFunction = funcModule;
	ir->addFunction(funcModule);
	currentBlock = currentFunction->getEntry();
	
	auto args = node->getArgs();
	for (auto &arg : args) arg->accept(*this);
	node->getBody()->accept(*this);

}

void IR_Generator::visit(ClassDecl * node)
{
}

void IR_Generator::visit(StmtBlock * node)
{
	auto stmts = node->getStmts();
	for (auto stmt : stmts) stmt->accept(*this);
}

void IR_Generator::visit(ExprStmt * node)
{
	node->getExpr()->accept(*this);
}

void IR_Generator::visit(BinaryExpr * node)
{
	auto op = node->getOperator();
	if (op == BinaryExpr::INDEX) arrayAccess(node);
	else if (op == BinaryExpr::ASSIGN) {
		node->getLHS()->accept(*this);
		return assign(node->getLHS()->getResultOprand(), node->getRHS().get());
	}
	// deal with assigning before bool
	else if (isBoolOnlyOperator(op)) return boolOnlyExpr(node);

	auto lhs = node->getLHS();
	auto rhs = node->getRHS();
	lhs->accept(*this);
	rhs->accept(*this);
	auto lhsReg = getValueReg(lhs->getResultOprand());
	auto rhsReg = getValueReg(rhs->getResultOprand());
	auto result = std::make_shared<VirtualReg>();
	node->setResultOprand(result);

	if (node->isString()) {  //  string op string, op = ==, != or +
		std::shared_ptr<Function> func;
		if (op == BinaryExpr::ADD) func = ir->stringAdd;
		else if (op == BinaryExpr::EQ) func = ir->stringEQ;
		else if (op == BinaryExpr::NEQ) func = ir->stringNEQ;
		auto call = std::make_shared<Call>(currentBlock, func, result);
		call->addArg(lhsReg); 
		call->addArg(rhsReg);
		currentBlock->append_back(call);
		return;
	}
	// integer op integer, bool == bool or bool != bool
	Quadruple::Operator instOp;
	switch (op)
	{
	case BinaryExpr::ADD: instOp = Quadruple::ADD;
		break;
	case BinaryExpr::MINUS: instOp = Quadruple::MINUS;
		break;
	case BinaryExpr::TIMES: instOp = Quadruple::TIMES;
		break;
	case BinaryExpr::DIVIDE: instOp = Quadruple::DIVIDE;
		break;
	case BinaryExpr::MOD: instOp = Quadruple::MOD;
		break;
	case BinaryExpr::LESS: instOp = Quadruple::LESS;
		break;
	case BinaryExpr::LEQ: instOp = Quadruple::LEQ;
		break;
	case BinaryExpr::GREATER: instOp = Quadruple::GREATER;
		break;
	case BinaryExpr::GEQ: instOp = Quadruple::GEQ;
		break;
	case BinaryExpr::NEQ: instOp = Quadruple::NEQ;
		break;
	case BinaryExpr::EQ: instOp = Quadruple::NEQ;
		break;
	case BinaryExpr::LSHIFT: instOp = Quadruple::LSHIFT;
		break;
	case BinaryExpr::RSHIFT: instOp = Quadruple::RSHIFT;
		break;
	case BinaryExpr::BITAND: instOp = Quadruple::BITAND;
		break;
	case BinaryExpr::BITOR: instOp = Quadruple::BITOR;
		break;
	case BinaryExpr::BITXOR: instOp = Quadruple::BITXOR;
		break;
	default:
		break;
	}
	currentBlock->append_back(std::make_shared<Quadruple>(currentBlock, instOp, result, lhsReg, rhsReg));
	if (node->isControl()) {
		currentBlock->endWith(std::make_shared<Branch>(
			currentBlock, result, node->getTrueBlock(), node->getFalseBlock()));
	}
}

void IR_Generator::visit(UnaryExpr * node)
{
	auto operand = node->getOperand();
	auto op = node->getOperator();
	if (op == UnaryExpr::NOT) {
		if (node->isControl()) {
			operand->setTrueBlock(node->getFalseBlock());
			operand->setFalseBlock(node->getTrueBlock());
			operand->accept(*this);
		}
		else {} // is this necessary ? 
		return;
	}
	Quadruple::Operator instOp;
	operand->accept(*this);
	switch (op)
	{
	case UnaryExpr::NEG: instOp = Quadruple::NEG;
		break;
	case UnaryExpr::INV: instOp = Quadruple::INV;
		break;
	case UnaryExpr::PREINC: instOp = Quadruple::ADD;
		break;
	case UnaryExpr::POSTINC: instOp = Quadruple::ADD;
		break;
	case UnaryExpr::PREDEC: instOp = Quadruple::MINUS;
		break;
	case UnaryExpr::POSTDEC: instOp = Quadruple::MINUS;
		break;
	default:
		break;
	}

	switch (op)
	{
	case UnaryExpr::POS:
		node->setResultOprand(getValueReg(operand->getResultOprand()));
		break;
	case UnaryExpr::NEG: // fall
	case UnaryExpr::INV: {
		auto result = std::make_shared<VirtualReg>();
		auto value = getValueReg(operand->getResultOprand());
		node->setResultOprand(result);
		currentBlock->append_back(std::make_shared<Quadruple>(
			currentBlock, instOp, result, value));
		break;
	}
	case UnaryExpr::PREINC: // fall
	case UnaryExpr::PREDEC: {
		auto result = operand->getResultOprand();
		auto value = getValueReg(result);
		node->setResultOprand(result);
		currentBlock->append_back(std::make_shared<Quadruple>(
			currentBlock, instOp, value, value, std::make_shared<Immediate>(1)));
		if (result->category() == Operand::REG_REF)
			currentBlock->append_back(std::make_shared<Quadruple>(
				currentBlock, Quadruple::STORE, result, value));
		break;
	}
	case UnaryExpr::POSTINC: // fall
	case UnaryExpr::POSTDEC: {
		auto before = operand->getResultOprand();
		auto value = getValueReg(before);
		auto temp = std::make_shared<VirtualReg>();
		// copy <value> to <temp>
		currentBlock->append_back(std::make_shared<Quadruple>(
			currentBlock, Quadruple::MOVE, temp, value));
		node->setResultOprand(temp);
		// <value> is updated
		currentBlock->append_back(std::make_shared<Quadruple>(
			currentBlock, instOp, value, value, std::make_shared<Immediate>(1)));
		if (before->category() == Operand::REG_REF)
			currentBlock->append_back(std::make_shared<Quadruple>(
				currentBlock, Quadruple::STORE, before, value));
		break;
	}
	default:
		break;
	}
}

void IR_Generator::arrayAccess(BinaryExpr * node)
{
	auto lhs = node->getLHS();
	auto rhs = node->getRHS();
	lhs->accept(*this);
	rhs->accept(*this);
	auto typeSymbol = std::static_pointer_cast<ArraySymbol>(lhs->getSymbolType());
	auto baseAddr = getValueReg(rhs->getResultOprand());
	auto index = getValueReg(rhs->getResultOprand());
	auto result = std::make_shared<VirtualReg>(Operand::REG_REF);
	auto elemSize = std::make_shared<Immediate>(typeSymbol->getElementSize());
	node->setResultOprand(result);

	auto offset = std::make_shared<VirtualReg>;
	currentBlock->append_back(std::make_shared<Quadruple>
		(currentBlock, Quadruple::TIMES, index,elemSize, offset));
	currentBlock->append_back(std::make_shared<Quadruple>
		(currentBlock, Quadruple::ADD, baseAddr, offset, result));

	if (node->isControl()) {
		// short-circuit evaluation for boolean value
		auto temp = std::make_shared<VirtualReg>();
		currentBlock->append_back(std::make_shared<Quadruple>(
			currentBlock,Quadruple::LOAD,result, temp));
		currentBlock->endWith(std::make_shared<Branch>
			(currentBlock, temp, node->getTrueBlock(), node->getFalseBlock()));
	}
}

void IR_Generator::boolOnlyExpr(BinaryExpr * node)
{
	if (node->isControl()) {  // short-circuit evaluation
		auto lhs = node->getLHS();
		auto rhs = node->getRHS();
		if (node->getOperator() == BinaryExpr::AND) {
			auto lhs_true = std::make_shared<BasicBlock>(currentFunction, BasicBlock::LHS_TRUE);
			lhs->setTrueBlock(lhs_true);
			lhs->setFalseBlock(node->getFalseBlock());
			lhs->accept(*this);
			currentBlock = lhs->getTrueBlock();
			rhs->setTrueBlock(node->getTrueBlock());
			rhs->setFalseBlock(node->getFalseBlock());
			rhs->accept(*this);
		}
		else { // OR
			auto lhs_false = std::make_shared<BasicBlock>(currentFunction, BasicBlock::LHS_FALSE);
			lhs->setFalseBlock(lhs_false);
			lhs->setTrueBlock(node->getTrueBlock());
			lhs->accept(*this);
			currentBlock = lhs->getFalseBlock();
			rhs->setTrueBlock(node->getTrueBlock());
			rhs->setFalseBlock(node->getFalseBlock());
			rhs->accept(*this);
		}
	}
	else { // is this necessary ?
		/*
		auto result = std::make_shared<VirtualReg>();
		node->setResultOprand(result);
		assignBool(node->getLHS()->getResultOprand(), node);
		*/
	}

}


void IR_Generator::assign(std::shared_ptr<Operand> lhs, Expression * rhsExpr)
{
	if (rhsExpr->isBool()) assignBool(lhs, rhsExpr);
	rhsExpr->accept(*this);
	auto rhsValReg = getValueReg(rhsExpr->getResultOprand());
	if (lhs->category() == Operand::REG_REF)
		currentBlock->append_back(std::make_shared<Quadruple>(
			currentBlock, Quadruple::STORE, lhs, rhsValReg));
	else currentBlock->append_back(std::make_shared<Quadruple>(
			currentBlock, Quadruple::MOVE, lhs, rhsValReg));
}

/*
Function : AssignBool
Note: lhs = bool is conducted as follows:
if bool == true
	lhs <- 1
else 
	lhs <- 0
*/
void IR_Generator::assignBool(std::shared_ptr<Operand> lhs, Expression *rhs)
{
	auto trueBlk = std::make_shared<BasicBlock>(currentBlock, BasicBlock::TRUE);
	auto falseBlk = std::make_shared<BasicBlock>(currentBlock, BasicBlock::FALSE);
	auto finalBlk = std::make_shared<BasicBlock>(currentBlock, BasicBlock::FINAL);
	rhs->setTrueBlock(trueBlk);
	rhs->setFalseBlock(falseBlk);
	rhs->accept(*this);
	auto op = lhs->category() == Operand::REG_REF ? Quadruple::STORE : Quadruple::MOVE;
	trueBlk->append_back(std::make_shared<Quadruple>(
		trueBlk, op, lhs, std::make_shared<Immediate>(1)));
	falseBlk->append_back(std::make_shared<Quadruple>(
		falseBlk, op, lhs, std::make_shared<Immediate>(0)));
}

bool IR_Generator::isBoolOnlyOperator(BinaryExpr::Operator op)
{
	return op == BinaryExpr::AND || op == BinaryExpr::OR;
}

/*
Function : getValueReg
Note: if <reg> is a reference(i.e. an address), load @<reg> into 
a register and return the new register. 
Otherwise, return <reg> directly.
*/
std::shared_ptr<Operand>IR_Generator::getValueReg(std::shared_ptr<Operand> reg)
{
	if (reg->category() != Operand::REG_REF) return reg;
	auto temp = std::make_shared<VirtualReg>(Operand::REG_VAL);
	currentBlock->append_back(std::make_shared<Quadruple>(
		currentBlock, Quadruple::LOAD, reg, temp));
	return temp;
}

