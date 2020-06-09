#include "IR_Generator.h"

void IR_Generator::visit(ProgramAST * node)
{
	auto decls = node->getDecls();
	scanGlobalVar = true;
	for (auto &decl : decls)
		if(decl->isVarDecl())decl->accept(*this);
		else if (decl->isFuncDecl()) {
			auto funcDeclNode = std::static_pointer_cast<FunctionDecl>(decl);
			funcDeclNode->getFuncSymbol()->setModule(newFunction(
				funcDeclNode->getFuncSymbol()->getSymbolName(), funcDeclNode->getRetType()));
		}
		else {
			auto clsNode = std::static_pointer_cast<ClassDecl>(decl);
			for (auto &member : clsNode->getMembers())
				if (!member->isVarDecl()) {
					auto symbol = std::static_pointer_cast<FunctionDecl>(member)->getFuncSymbol();
					auto retType = std::static_pointer_cast<FunctionDecl>(member)->getRetType();
					auto oldName = symbol->getSymbolName();

					auto funcName = "_" + clsNode->getClsSymbol()->getSymbolName()
						+ "_" + oldName;

					for (int i = 0; i < oldName.length(); i++)
						if (oldName[i] == ':') {  // constructor;
							funcName = "__ctor_" + clsNode->getClsSymbol()->getSymbolName(); 
							break;
						}

					symbol->setModule(newFunction(funcName, retType));
				}
		}
	scanGlobalVar = false;

	// function moduel must be initialized here, since declaration might come after usage.
	for (auto &decl : decls)
		if (!decl->isVarDecl()) decl->accept(*this);
}


void IR_Generator::generate()
{
	builtInFunctionInit();
	visit(ast.get());
}

void IR_Generator::builtInFunctionInit()
{
	std::shared_ptr<FunctionSymbol> symbol;
	// member functions of class <string>
	symbol = std::static_pointer_cast<FunctionSymbol>(global->getStringSymbol()->resolve("length"));
	symbol->setModule(ir->stringLength);

	symbol = std::static_pointer_cast<FunctionSymbol>(global->getStringSymbol()->resolve("parseInt"));
	symbol->setModule(ir->parseInt);

	symbol = std::static_pointer_cast<FunctionSymbol>(global->getStringSymbol()->resolve("substring"));
	symbol->setModule(ir->substring);

	symbol = std::static_pointer_cast<FunctionSymbol>(global->getStringSymbol()->resolve("ord"));
	symbol->setModule(ir->ord);

	symbol = std::static_pointer_cast<FunctionSymbol>(global->resolve("getString"));
	symbol->setModule(ir->getString);

	symbol = std::static_pointer_cast<FunctionSymbol>(global->resolve("print"));
	symbol->setModule(ir->print);

	symbol = std::static_pointer_cast<FunctionSymbol>(global->resolve("println"));
	symbol->setModule(ir->println);

	//functions about <int>
	symbol = std::static_pointer_cast<FunctionSymbol>(global->resolve("getInt"));
	symbol->setModule(ir->getInt);

	symbol = std::static_pointer_cast<FunctionSymbol>(global->resolve("toString"));
	symbol->setModule(ir->toString);

	symbol = std::static_pointer_cast<FunctionSymbol>(global->resolve("printInt"));
	symbol->setModule(ir->printInt);

	symbol = std::static_pointer_cast<FunctionSymbol>(global->resolve("printlnInt"));
	symbol->setModule(ir->printlnInt);
}

void IR_Generator::visit(MultiVarDecl * node)
{
	auto vars = node->getDecls();
	for (auto &var : vars) var->accept(*this);
}

void IR_Generator::visit(VarDeclStmt * node)
{
	auto varSymbol = node->getVarSymbol();
	std::shared_ptr<VirtualReg> reg;
	reg = std::make_shared<VirtualReg>(Operand::REG_VAL,node->getIdentifier()->name);
	varSymbol->setReg(reg);
	if (scanGlobalVar) {  // record global variable so that we can print it later 
		ir->addGlobalVar(reg);
		reg->markAsGlobal();
	}
	else {
		if (currentFunction != nullptr && node->isArgument()) {  // add argument to function module 
			currentFunction->appendArg(reg);
		}
		auto initExpr = node->getInitExpr();
		if (initExpr != nullptr) {
			//initExpr->accept(*this);
			assign(reg, initExpr.get());
		}
	}
}

void IR_Generator::visit(FunctionDecl * node)   // Function module is linked to function symbol here
{
	auto symbol = node->getFuncSymbol();
	if (currentClsSymbol != nullptr) {
		symbol->getModule()->setObjRef(std::make_shared<VirtualReg>(Operand::REG_VAL, "this"));
	}

	currentFunction = symbol->getModule();
	currentBlock = currentFunction->getEntry();
	auto args = node->getArgs();
	for (auto &arg : args) arg->accept(*this);
	node->getBody()->accept(*this);

	if (!currentBlock->ended()) {  // no return instruction
		if (node->getRetType() == nullptr)
			currentBlock->endWith(std::make_shared<Return>(currentBlock, nullptr));
		else
			currentBlock->endWith(std::make_shared<Return>(currentBlock, std::make_shared<Immediate>(0)));
	}
	mergeReturnIntoExit(node, currentFunction);

	ir->addFunction(currentFunction);
	//build dominance tree
	currentFunction->setDT(std::make_shared<DominatorTree>(currentFunction));
	
	currentFunction = nullptr;
}

/*
Note : 
1. Set fucntion modules for member functions;
2. Maintain <currentClsSymbol>;
3. Declaretion of mamber varibles are of no significance here.
*/
void IR_Generator::visit(ClassDecl * node)
{
	currentClsSymbol = node->getClsSymbol();	
	for (auto &member : node->getMembers())
		if (!member->isVarDecl()) member->accept(*this);
	currentClsSymbol = nullptr;
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

void IR_Generator::visit(ForStmt * node)
{
	if (node->getInit() != nullptr) node->getInit()->accept(*this);
	auto cond = node->getCondition();
	auto iter = node->getIter();

	auto bodyBlk = std::make_shared<BasicBlock>(currentFunction, BasicBlock::FOR_BODY);
	auto finalBlk = std::make_shared<BasicBlock>(currentFunction, BasicBlock::FOR_FINAL);
	auto condBlk = cond == nullptr ? 
		bodyBlk : std::make_shared<BasicBlock>(currentFunction, BasicBlock::FOR_COND);
	auto iterBlk = iter == nullptr ?
		condBlk : std::make_shared<BasicBlock>(currentFunction, BasicBlock::FOR_ITER);
	node->setStartBlk(condBlk);
	node->setFinalBlk(finalBlk);
	// Jump to condition if there is, or else jump to body.
	currentBlock->endWith(std::make_shared<Jump>(currentBlock, condBlk));
	if (cond != nullptr) {
		currentBlock = condBlk;
		cond->setTrueBlock(bodyBlk);
		cond->setFalseBlock(finalBlk);  // jump out of loop
		cond->accept(*this);
	}

	currentBlock = bodyBlk;
	node->getBody()->accept(*this);

	if (!currentBlock->ended())
		currentBlock->endWith(std::make_shared<Jump>(currentBlock, iterBlk));
	if (iter != nullptr) {
		currentBlock = iterBlk;
		iter->accept(*this);
		if (!currentBlock->ended())
			currentBlock->endWith(std::make_shared<Jump>(currentBlock, condBlk));
	}
	currentBlock = finalBlk;
}

void IR_Generator::visit(WhileStmt * node)
{
	auto bodyBlk = std::make_shared<BasicBlock>(currentFunction, BasicBlock::WHILE_BODY);
	auto finalBlk = std::make_shared<BasicBlock>(currentFunction, BasicBlock::WHILE_FINAL);
	
	auto cond = node->getCondition();
	auto condBlk = std::make_shared<BasicBlock>(currentFunction, BasicBlock::WHILE_COND);
	node->setStartBlk(condBlk);
	node->setFinalBlk(finalBlk);

	currentBlock->endWith(std::make_shared<Jump>(currentBlock, condBlk));
	currentBlock = condBlk;
	cond->setTrueBlock(bodyBlk);
	cond->setFalseBlock(finalBlk);
	cond->accept(*this);

	currentBlock = bodyBlk;
	node->getBody()->accept(*this);
	if (!currentBlock->ended())
		currentBlock->endWith(std::make_shared<Jump>(currentBlock, condBlk));
	currentBlock = finalBlk;
}

void IR_Generator::visit(IfStmt * node)
{
	auto trueBlk = std::make_shared<BasicBlock>(currentFunction, BasicBlock::IF_TRUE);
	auto finalBlk = std::make_shared<BasicBlock>(currentFunction, BasicBlock::IF_FINAL);

	auto cond = node->getCondition();
	cond->setTrueBlock(trueBlk);

	auto _else = node->getElse();
	if (_else == nullptr) {
		cond->setFalseBlock(finalBlk);
		cond->accept(*this);
		currentBlock = trueBlk;
		node->getThen()->accept(*this);
		if (!currentBlock->ended())
			currentBlock->endWith(std::make_shared<Jump>(currentBlock, finalBlk));
	}
	else {
		auto falseBlk = std::make_shared<BasicBlock>(currentFunction, BasicBlock::IF_FALSE);
		cond->setFalseBlock(falseBlk);
		cond->accept(*this);
		currentBlock = trueBlk;
		node->getThen()->accept(*this);
		if (!currentBlock->ended())
			currentBlock->endWith(std::make_shared<Jump>(currentBlock, finalBlk));
		currentBlock = falseBlk;
		_else->accept(*this);
		if (!currentBlock->ended())
			currentBlock->endWith(std::make_shared<Jump>(currentBlock, finalBlk));
	}
	currentBlock = finalBlk;
}

void IR_Generator::visit(ReturnStmt * node)
{
	if (node->getValue() == nullptr)
		currentBlock->endWith(std::make_shared<Return>(currentBlock, nullptr));
	else {
		auto reg = std::make_shared<VirtualReg>();
		assign(reg, node->getValue().get());
		currentBlock->endWith(std::make_shared<Return>(currentBlock, reg));
	}
}

void IR_Generator::visit(ContinueStmt * node)
{
	currentBlock->endWith(std::make_shared<Jump>(currentBlock, node->getLoop()->getStartBlk()));
}

void IR_Generator::visit(BreakStmt * node)
{
	currentBlock->endWith(std::make_shared<Jump>(currentBlock, node->getLoop()->getFinalBlk()));
}


//Note: resolve offset.
void IR_Generator::visit(IdentifierExpr * node)
{
	auto symbol = node->getSymbol();
	if (symbol->getScope() == currentClsSymbol) {
		// x is equivalent to this.x
		if (symbol->category() == Symbol::VAR) {
			auto ref = std::make_shared<VirtualReg>(Operand::REG_REF);
			auto offset = std::make_shared<Immediate>(std::static_pointer_cast<VarSymbol>(symbol)->getOffset());
			currentBlock->append_back(std::make_shared<Quadruple>(
				currentBlock, Quadruple::ADD, ref, currentFunction->getObjRef(), offset));
			node->setResultOprand(ref);
			if (node->isControl()) {
				auto value = getValueReg(ref);
				currentBlock->endWith(std::make_shared<Branch>(
					currentBlock, value, node->getTrueBlock(), node->getFalseBlock()));
			}
		}
		else
			node->setResultOprand(currentFunction->getObjRef());
	}
	else if (symbol->category() == Symbol::VAR) { 
		// for normal variable, return its reference directly
		auto result = std::static_pointer_cast<VarSymbol>(symbol)->getReg();
		node->setResultOprand(result);
		if (node->isControl())
			currentBlock->endWith(std::make_shared<Branch>(
				currentBlock, result, node->getTrueBlock(), node->getFalseBlock()));
	}
}

void IR_Generator::visit(BinaryExpr * node)
{
	auto op = node->getOperator();
	if (op == BinaryExpr::INDEX) return arrayAccess(node);
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
		else if (op == BinaryExpr::LESS) func = ir->stringLESS;
		else if (op == BinaryExpr::LEQ) func = ir->stringLEQ;
		else if (op == BinaryExpr::GREATER) func = ir->stringGREATER;
		else if (op == BinaryExpr::GEQ) func = ir->stringGEQ;
		auto call = std::make_shared<Call>(currentBlock, func, result);
		call->addArg(lhsReg); 
		call->addArg(rhsReg);
		currentBlock->append_back(call);
		return;
	}
	else {
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
		case BinaryExpr::EQ: instOp = Quadruple::EQ;
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
	}
	if (node->isControl()) { // This is only for comparison between integers or strings
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
		else {
			operand->setResultOprand(std::make_shared<VirtualReg>());
			assign(operand->getResultOprand(), node);
		} 
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

void IR_Generator::visit(FuncCallExpr * node)
{
	auto func = node->getFuncSymbol();
	std::shared_ptr<Operand> result;
	if (node->getSymbolType()->getTypeName() == "void")
		result = nullptr;
	else result = std::make_shared<VirtualReg>();
	node->setResultOprand(result);

	auto call = std::make_shared<Call>(currentBlock, func->getModule(), result);
	auto args = node->getArgs();
	for (auto &arg : args) {
		arg->accept(*this);
		call->addArg(getValueReg(arg->getResultOprand()));
	}

	if(currentClsSymbol ==  func->getEnclosingScope()) 
		call->setObjRef(currentFunction->getObjRef()); // a class function
	currentBlock->append_back(call);
	lastCall = call;
	if (node->isControl())
		currentBlock->endWith(std::make_shared<Branch>(
			currentBlock, result, node->getTrueBlock(), node->getFalseBlock()));
}

void IR_Generator::visit(MemberFuncCallExpr * node)
{
	if (!node->getInstance()->isObject())
		return arraySize(node);
	node->getInstance()->accept(*this);
	auto func = node->getFuncSymbol();
	std::shared_ptr<Operand> result;
	if (node->getSymbolType()->getTypeName() == "void")
		result = nullptr;
	else result = std::make_shared<VirtualReg>();
	node->setResultOprand(result);

	auto call = std::make_shared<Call>(currentBlock, func->getModule(), result);
	auto args = node->getArgs();
	for (auto &arg : args) {
		arg->accept(*this);
		call->addArg(getValueReg(arg->getResultOprand()));
	}
	call->setObjRef(node->getInstance()->getResultOprand());
	currentBlock->append_back(call);
	if (node->isControl())
		currentBlock->endWith(std::make_shared<Branch>(
			currentBlock, result, node->getTrueBlock(), node->getFalseBlock()));
}

/*
Note: To access a member variable, we need 
store the offset of each variable in corresponding
symbol in advance.
*/
void IR_Generator::visit(ClassMemberExpr * node)
{
	node->getObj()->accept(*this);
	auto base = getValueReg(node->getObj()->getResultOprand());
	auto symbol = std::static_pointer_cast<VarSymbol>(node->getSymbol());
	auto ref = std::make_shared<VirtualReg>(Operand::REG_REF);
	auto offset = std::make_shared<Immediate>(symbol->getOffset());
	node->setResultOprand(ref);
	currentBlock->append_back(std::make_shared<Quadruple>(
		currentBlock, Quadruple::ADD, ref, base, offset));

	if (node->isControl())
		currentBlock->endWith(std::make_shared<Branch>(
			currentBlock, getValueReg(ref), node->getTrueBlock(), node->getFalseBlock()));
}

void IR_Generator::visit(ThisExpr * node)
{
	node->setResultOprand(currentFunction->getObjRef());
}

/*
Note : note that builtinTypes cannot use <new>, only object and array are allowed.
*/
void IR_Generator::visit(NewExpr * node)
{
	auto result = std::make_shared<VirtualReg>();
	node->setResultOprand(result);
	if (node->getSymbolType()->isArrayType()) newArray(node,node->getResultOprand());
	else { // object
		//1. allocate memory
		auto clsSymbol = std::static_pointer_cast<ClassSymbol>(node->getSymbolType());
		auto objSize = std::make_shared<Immediate>(clsSymbol->getSize());
		currentBlock->append_back(std::make_shared<Malloc>(currentBlock, objSize, result));

		if (node->getCtorCall() != nullptr && clsSymbol->getConstructor() != nullptr) {
			node->getCtorCall()->accept(*this);
			lastCall->setObjRef(result);
		}
		else if (clsSymbol->getConstructor() != nullptr) {
			//2. call constructor
			if (clsSymbol->getConstructor()->getFormalArgs().empty()) {
				auto call = std::make_shared<Call>(currentBlock, clsSymbol->getConstructor()->getModule());
				call->setObjRef(node->getResultOprand());
				currentBlock->append_back(call);
			}
		}
	}
}

void IR_Generator::visit(NumValue * node)
{
	node->setResultOprand(std::make_shared<Immediate>(node->getValue()));
}

void IR_Generator::visit(BoolValue * node)
{
	node->setResultOprand(std::make_shared<Immediate>(node->getValue()));
	if (node->isControl()) {
		std::shared_ptr<Jump> jump;
		if (node->getValue())
			jump = std::make_shared<Jump>(currentBlock, node->getTrueBlock());
		else jump = std::make_shared<Jump>(currentBlock, node->getFalseBlock());
		currentBlock->endWith(jump);
	}
}

void IR_Generator::visit(StringValue * node)
{
	auto reg = std::make_shared<VirtualReg>(Operand::REG_VAL, "__str");  // 
	auto str = std::make_shared<StaticString>(reg, node->getText().substr(1, node->getText().length() - 2)); //get rid of ''
	node->setResultOprand(reg);  // reg or str?
	reg->markAsStaticString();
	ir->addStringConst(str);
}

void IR_Generator::visit(NullValue * node)
{
	// is this necessary?
	node->setResultOprand(std::make_shared<Immediate>(0));
}

/*
Note: The size of array is stored at the begining address.
Hence,  memorySize = N * elementSize + <regSize>.
*/
void IR_Generator::newArray(NewExpr * node, std::shared_ptr<Operand> addrReg, int dimension)
{
	auto sizeExpr = (node->getDimensions())[dimension];
	sizeExpr->accept(*this);
	auto N_reg = getValueReg(sizeExpr->getResultOprand());
	auto size = std::make_shared<VirtualReg>();
	auto size_of_int = std::make_shared<Immediate>(Configuration::SIZE_OF_INT);
	auto size_of_ptr = std::make_shared<Immediate>(Configuration::SIZE_OF_PTR);

	if (dimension == node->getDimensions().size() - 1) {	// the last dimension
		//1. calaulate <size>.
		auto elemSize = std::make_shared<Immediate>(
			dimension == node->getNumberOfDim() - 1 ?
			std::static_pointer_cast<ArraySymbol>(node->getSymbolType())->getBaseType()->getSize()
			: Configuration::SIZE_OF_PTR);
		currentBlock->append_back(std::make_shared<Quadruple>(
			currentBlock, Quadruple::TIMES, size, elemSize, N_reg));
		currentBlock->append_back(std::make_shared<Quadruple>(
			currentBlock, Quadruple::ADD, size, size, size_of_int));
		// 2. Allocate memory.
		allocateMemory(addrReg,size, N_reg);
	}
	else {  // elemSize = size_of_ptr
		//1. Allocate memory for pointers.
		currentBlock->append_back(std::make_shared<Quadruple>(
			currentBlock, Quadruple::TIMES, size, size_of_ptr, N_reg));
		currentBlock->append_back(std::make_shared<Quadruple>(
			currentBlock, Quadruple::ADD, size, size,size_of_int));
		auto arrayAddr = allocateMemory(addrReg, size, N_reg);
		//2.Use a for loop to allocate memory for each pointers in the array.
		auto bodyBlk = std::make_shared<BasicBlock>(currentFunction, BasicBlock::FOR_BODY);
		auto finalBlk = std::make_shared<BasicBlock>(currentFunction, BasicBlock::FOR_FINAL);
		auto condBlk = std::make_shared<BasicBlock>(currentFunction, BasicBlock::FOR_COND);
		auto iterBlk = std::make_shared<BasicBlock>(currentFunction, BasicBlock::FOR_ITER);
		// 3. Calaulate the begining address and upperbound.
		auto curAddr = std::make_shared<VirtualReg>(Operand::REG_REF);

		auto upperBound = std::make_shared<VirtualReg>();
		currentBlock->append_back(std::make_shared<Quadruple>(
			currentBlock, Quadruple::ADD, curAddr, arrayAddr,size_of_int));
		currentBlock->append_back(std::make_shared<Quadruple>(
			currentBlock, Quadruple::ADD, upperBound, arrayAddr, size));
		currentBlock->endWith(std::make_shared<Jump>(currentBlock, condBlk));
		// 4. condtion : curAddr < upperBound
		auto cmp = std::make_shared<VirtualReg>();
		condBlk->append_back(std::make_shared<Quadruple>(
			condBlk, Quadruple::LESS, cmp, curAddr, upperBound));
		condBlk->endWith(std::make_shared<Branch>(condBlk, cmp, bodyBlk, finalBlk));
		// 5. Body : curAddr = new array (recursively)
		currentBlock = bodyBlk;
		newArray(node, curAddr, dimension + 1);
		currentBlock->endWith(std::make_shared<Jump>(currentBlock, iterBlk));
		// 6. Iteration : curAddr += size_of_ptr
		iterBlk->append_back(std::make_shared<Quadruple>(
			iterBlk, Quadruple::ADD, curAddr, size_of_ptr, curAddr));
		iterBlk->endWith(std::make_shared<Jump>(iterBlk, condBlk));
		currentBlock = finalBlk;
	}
}

//Usage: addrValReg = allocateMemory(addr,size)
std::shared_ptr<Operand>
IR_Generator::allocateMemory(std::shared_ptr<Operand> addrReg, std::shared_ptr<Operand> size, std::shared_ptr<Operand> numOfObj)
{
	if (addrReg->category() == Operand::REG_REF) {
		auto temp = std::make_shared<VirtualReg>();
		currentBlock->append_back(std::make_shared<Malloc>(currentBlock, size, temp));
		//store size to the begining address;
		currentBlock->append_back(std::make_shared<Quadruple>(currentBlock, Quadruple::STORE, temp, numOfObj));
		currentBlock->append_back(std::make_shared<Quadruple>(currentBlock, Quadruple::STORE, addrReg, temp));
		return temp;
	}
	else {
		currentBlock->append_back(std::make_shared<Malloc>(currentBlock, size, addrReg));
		currentBlock->append_back(std::make_shared<Quadruple>(currentBlock, Quadruple::STORE, addrReg, numOfObj));
		return addrReg;
	}
}

void IR_Generator::mergeReturnIntoExit(FunctionDecl *node, std::shared_ptr<Function> f)
{
	auto rets = f->getReturnIntrs();
	if (rets.size() > 1) {
		auto exit = f->getExit();
		auto result = node->getRetType() == nullptr ? nullptr : std::make_shared<VirtualReg>();

		for (auto &ret : rets) {
			removeInstruction(ret);
			if (ret->getValue() != nullptr)
				ret->getBlock()->append_back(std::make_shared<Quadruple>(
					ret->getBlock(), Quadruple::MOVE, result, ret->getValue()));
			ret->getBlock()->endWith(std::make_shared<Jump>(ret->getBlock(), exit));
			ret->getBlock()->link_to_block(exit);
		}

		auto new_ret = std::make_shared<Return>(exit, result);
		exit->endWith(new_ret);
		f->getReturnIntrs().clear();
		f->getReturnIntrs().push_back(new_ret);   // remember renewing this!!
	}
	else f->setExit(rets[0]->getBlock());
}


void IR_Generator::arraySize(MemberFuncCallExpr * node)
{
	node->getInstance()->accept(*this);
	auto ref = getValueReg(node->getInstance()->getResultOprand());
	auto result = std::make_shared<VirtualReg>();
	node->setResultOprand(result);
	currentBlock->append_back(std::make_shared<Quadruple>(
		currentBlock, Quadruple::LOAD, result, ref));
}

void IR_Generator::arrayAccess(BinaryExpr * node)
{
	auto lhs = node->getLHS();
	auto rhs = node->getRHS();
	lhs->accept(*this);
	rhs->accept(*this);
	auto typeSymbol = std::static_pointer_cast<ArraySymbol>(lhs->getSymbolType());
	auto baseAddr = getValueReg(lhs->getResultOprand());
	auto index = getValueReg(rhs->getResultOprand());
	// the result is a reference(i.e. address in RAM)
	auto result = std::make_shared<VirtualReg>(Operand::REG_REF);
	auto elemSize = std::make_shared<Immediate>(typeSymbol->getElementSize());
	node->setResultOprand(result);

	auto offset = std::make_shared<VirtualReg>();
	auto temp = std::make_shared<VirtualReg>();
	auto reg_size = std::make_shared<Immediate>(Configuration::SIZE_OF_INT);
	currentBlock->append_back(std::make_shared<Quadruple>
		(currentBlock, Quadruple::TIMES, temp, index,elemSize));
	currentBlock->append_back(std::make_shared<Quadruple>
		(currentBlock, Quadruple::ADD, offset, temp, reg_size));
	// note: their is a register at the begining to store size of array.
	currentBlock->append_back(std::make_shared<Quadruple>
		(currentBlock, Quadruple::ADD, result, offset, baseAddr));
	if (node->isControl()) {
		// short-circuit evaluation for boolean value
		auto temp = std::make_shared<VirtualReg>();
		currentBlock->append_back(std::make_shared<Quadruple>(
			currentBlock,Quadruple::LOAD,temp, result));
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
		auto result = std::make_shared<VirtualReg>();
		node->setResultOprand(result);
		assignBool(node->getResultOprand(), node);
	}

}


void IR_Generator::assign(std::shared_ptr<Operand> lhs, Expression * rhsExpr)
{
	if (rhsExpr->isBool()) return assignBool(lhs, rhsExpr);
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
	auto trueBlk = std::make_shared<BasicBlock>(currentFunction, BasicBlock::TRUE);
	auto falseBlk = std::make_shared<BasicBlock>(currentFunction, BasicBlock::FALSE);
	auto finalBlk = std::make_shared<BasicBlock>(currentFunction, BasicBlock::FINAL);
	rhs->setTrueBlock(trueBlk);
	rhs->setFalseBlock(falseBlk);
	rhs->accept(*this);
	auto op = lhs->category() == Operand::REG_REF ? Quadruple::STORE : Quadruple::MOVE;
	trueBlk->append_back(std::make_shared<Quadruple>(
		trueBlk, op, lhs, std::make_shared<Immediate>(1)));
	falseBlk->append_back(std::make_shared<Quadruple>(
		falseBlk, op, lhs, std::make_shared<Immediate>(0)));

	trueBlk->endWith(std::make_shared<Jump>(trueBlk, finalBlk));
	falseBlk->endWith(std::make_shared<Jump>(falseBlk, finalBlk));
	currentBlock = finalBlk;
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
		currentBlock, Quadruple::LOAD, temp, reg));
	return temp;
}

