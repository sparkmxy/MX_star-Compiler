#include "parser.h"
#include "token.h"

std::shared_ptr<Identifier> Parser::identifier() { 
	if ((*cur)->tag() == ID) {
		auto p = (*cur)->pos();
		return newNode<Identifier>(p,(*(cur++))->toString());
	}
	return nullptr;
}

std::shared_ptr<BuiltinType> Parser::builtinType() {
	auto id = (*cur)->tag();
	if (id == Num || id == String || id == Bool) {
		auto p = (*cur)->pos();
		switch (id) {
		case Num: return newNode<BuiltinType>(p, BuiltinType::INT);
		case String: return newNode<BuiltinType>(p,  BuiltinType::STRING);
		case Bool: return newNode<BuiltinType>(p,  BuiltinType::BOOL);
		default:
			throw SyntaxError("Parser error: undefined builtin-type.",(*cur)->pos().first);
		}
	}
	return nullptr;
}

std::shared_ptr<BasicType> Parser::basicType() {
	std::shared_ptr<BasicType> ret = builtinType();
	if (ret != nullptr) return ret;
	auto id = identifier();
	if (id != nullptr) return newNode<UserDefinedType>(id->getPos(), id);
	return nullptr;
}

std::shared_ptr<Type> Parser::type() {
	auto st = cur;
	std::shared_ptr<Type> ret = basicType();
	if (ret == nullptr) return nullptr;
	while ((*cur)->tag() == LeftIndex) {
		cur++;
		if ((*cur)->tag() == RightIndex) 
			ret = newNode<ArrayType>((*st)->pos().first, (*cur)->pos().second, ret);
		else{
			cur = st;
			return nullptr;
		}
		cur++;
	}
	return ret;
}

/**************************************Types ends here*******************************/

std::shared_ptr<ConstValue> Parser::constValue() {
	auto tag = (*cur)->tag();
	if (tag == Num || tag == True || tag == False || tag == ConstString || tag == Null) {
		auto p = (*cur)->pos(); 
		Token *tkptr = *cur;
		cur++;
		switch (tag)
		{
		case Num: return newNode<NumValue>(p,reinterpret_cast<Number*>(tkptr)->value());
		case ConstString: return newNode<StringValue>(p, tkptr->toString());
		case True: return newNode<BoolValue>(p, true);
		case False: return newNode<BoolValue>(p, false);
		case Null: return newNode<NullValue>(p);
		}
	}
	return nullptr;
}

std::shared_ptr<NewExpr> Parser::newExpr() {
	auto st = (*cur)->pos().first;
	if ((*cur)->tag() != New) return nullptr;
	cur++;
	
	std::shared_ptr<Type> baseType = basicType();
	if (baseType == nullptr) return nullptr;
	std::vector<std::shared_ptr<Expression>> dimensions;

	//()
	if ((*cur)->tag() == LeftBracket) {
		cur++;
		if ((*cur)->tag() != RightBracket)
			throw SyntaxError("Parser error : Missing ')' in new expression", (*cur)->pos().first);
		return newNode<NewExpr>(st, (*(cur++))->pos().second, baseType, dimensions);
	}

	bool noDim = false;
	while ((*cur)->tag() == LeftIndex) {
		cur++;
		if ((*cur)->tag() == RightIndex) {  // no more dimensions
			noDim = true;
			cur++;
			baseType = newNode<ArrayType>(st, (*cur)->pos().second, baseType);
			continue;
		}
		if (noDim)
			throw SyntaxError("no dimension declaration",(*cur)->pos().first);
		auto expr = expression();
		if (expr == nullptr)
			throw SyntaxError("Parser error: incomplete expression", (*cur)->pos().first);
		dimensions.push_back(expr);
	}
	return newNode<NewExpr>(st, (*cur)->pos().second, baseType, dimensions);
}

std::shared_ptr<Expression> Parser::identifierExpr() {
	auto ret = identifier();
	if (ret == nullptr) return nullptr;
	return newNode<IdentifierExpr>(ret->getPos(),ret);
}

std::shared_ptr<Expression> Parser::basicExpr() {
	auto st = (*cur)->pos().first;
	Tag tag = (*cur)->tag();
	if (tag == ID) {
		// identifierExpr or FunctionCall
		auto idExpr = std::static_pointer_cast<IdentifierExpr>(identifierExpr());
		if ((*cur)->tag() == LeftBracket) {
			// function call
			auto args = arguments();
			if ((*cur)->tag() != RightBracket)
				throw SyntaxError("Parser error : missing ')'.", (*cur)->pos().first);
			return newNode<FuncCallExpr>(st, (*(cur++))->pos().second, idExpr, args);
		}
		return idExpr;
	}
	else if (tag == New) {
		// newExpr
		return newExpr();
	}
	else if (tag == LeftBracket) {
		// (expr)
		cur++;
		auto ret = expression();
		if ((*cur)->tag() != RightBracket) return nullptr;
		cur++;
		return ret;
	}
	else {
		// ConstValue;
		return constValue();
	}
}

std::shared_ptr<Expression> Parser::topPriorityExpr() {
	auto st = (*cur)->pos().first;

	auto lhs = basicExpr();
	if (lhs == nullptr) return nullptr;

	while (true) {
		if ((*cur)->tag() == LeftIndex) {
			// index operation e.g. a[i+1]
			cur++;
			auto rhs = expression();
			if (rhs == nullptr)
				throw SyntaxError("Parser error: missing expression.",(*cur)->pos().first);
			if((*cur)->tag() != RightIndex)
				throw SyntaxError("Parser error: missing ']'", (*cur)->pos().first);
			return newNode<BinaryExpr>(st, (*cur)->pos().second, BinaryExpr::INDEX, lhs, rhs);
		}
		else if ((*cur)->tag() == Domain){
			cur++;
			auto identifier = std::static_pointer_cast<IdentifierExpr>(identifierExpr());
			if (!identifier)
				throw SyntaxError("Parser error: miss identifier." ,(*cur)->pos().first);
			if ((*cur)->tag() == LeftBracket) {
				// fucntion call with domain
				cur++;
				auto args = arguments();
				if ((*cur)->tag() != RightBracket)
					throw SyntaxError("Parser error: miss ')'.", (*cur)->pos().first);
				return newNode<MemberFuncCallExpr>(st,(*(cur++))->pos().second, lhs, identifier, args);
			}
			else {
				// identifier in domain
				return newNode<ClassMemberExpr>(st, (*cur)->pos().second, lhs, identifier);
			}
		}
	}
}


std::shared_ptr<Expression> Parser::suffixUnaryExpr() {
	// ++ --
	auto st = (*cur)->pos().first;
	std::shared_ptr<Expression> ret = topPriorityExpr();
	if (ret == nullptr) return nullptr;
	if ((*cur)->tag() == Inc) {
		ret = newNode<UnaryExpr>(st, (*cur)->pos().second, UnaryExpr::POSTINC, ret);
		cur++;
	}
	else if((*cur)->tag() == Dec){
		ret = newNode<UnaryExpr>(st, (*cur)->pos().second, UnaryExpr::POSTDEC, ret);
		cur++;
	}
	return ret;
}

std::shared_ptr<Expression> Parser::prefixUnaryExpr() {
	/*
		++ -- + - ! ~ 
	*/
	auto st = (*cur)->pos().first;
	
	std::vector<UnaryExpr::Operator> prefixOp;
	bool flag = true;
	while (flag) {
		switch ((*(cur++))->tag())
		{
		case Inc: prefixOp.push_back(UnaryExpr::PREINC); break;
		case Dec: prefixOp.push_back(UnaryExpr::PREDEC); break;
		case Add: prefixOp.push_back(UnaryExpr::POS); break;
		case Minus: prefixOp.push_back(UnaryExpr::NEG); break;
		case Not: prefixOp.push_back(UnaryExpr::NOT); break;
		case Inverse: prefixOp.push_back(UnaryExpr::INV); break;
		default: 
			cur--;
			flag = false;
			break; 
		}
	}

	auto ret = suffixUnaryExpr();
	if (ret == nullptr) {
		if (prefixOp.empty()) return nullptr;
		throw SyntaxError("Parser error: incomplete prefix expression.", (*cur)->pos().first);
	}

	for (auto &op : prefixOp)
		ret = newNode<UnaryExpr>(st,ret->endPos(),op,ret);
	return ret;
}

std::shared_ptr<Expression> Parser::multiplicativeExpr() {
	return
		expressionHelper(std::bind(&Parser::prefixUnaryExpr, this),
			op.multiplicative());
}

std::shared_ptr<Expression> Parser::additiveExpr() {
	return
		expressionHelper(std::bind(&Parser::multiplicativeExpr, this),
			op.additive());
}

std::shared_ptr<Expression> Parser::shiftExpr() {
	return
		expressionHelper(std::bind(&Parser::additiveExpr, this),
			op.shift());
}


std::shared_ptr<Expression> Parser::relationExpr() {
	return expressionHelper(std::bind(&Parser::shiftExpr, this),op.rel());
}

std::shared_ptr<Expression> Parser::equalityExpr() {
	return
		expressionHelper(std::bind(&Parser::relationExpr, this),
			op.equality());
}

std::shared_ptr<Expression> Parser::bitAndExpr() {
	return expressionHelper(std::bind(&Parser::equalityExpr, this),
			std::make_pair(BitAnd, BinaryExpr::Operator::BITAND));
}

std::shared_ptr<Expression> Parser::bitXorExpr() {
	return expressionHelper(std::bind(&Parser::bitAndExpr, this),
			std::make_pair(Xor, BinaryExpr::Operator::BITXOR));
}


std::shared_ptr<Expression> Parser::bitOrExpr() {
	return expressionHelper(std::bind(&Parser::bitXorExpr, this),
			std::make_pair(BitOr, BinaryExpr::Operator::BITOR));
}
      
std::shared_ptr<Expression> Parser::logicalAndExpr() {
	return expressionHelper(std::bind(&Parser::bitOrExpr, this),
			std::make_pair(BitAnd, BinaryExpr::Operator::BITAND));
}

std::shared_ptr<Expression> Parser::logicalOrExpr() {
	return expressionHelper(std::bind(&Parser::logicalAndExpr, this),
			std::make_pair(And, BinaryExpr::Operator::AND));
}

std::shared_ptr<Expression> Parser::expression() {
	return expressionHelper(std::bind(&Parser::logicalOrExpr, this),
			std::make_pair(Assign, BinaryExpr::Operator::ASSIGN));
}
/******************************************Expressions end here************************************************/

std::shared_ptr<VarDeclStmt> Parser::varDeclStmt() {
	auto st = (*cur)->pos().first;
	auto iter = cur;

	auto _type = type();
	if (_type == nullptr) { cur = iter; return nullptr; }

	auto id = identifier();
	if (id == nullptr) { cur = iter; return nullptr; }

	Tag nextToken = (*cur)->tag();
	if (nextToken == Semicolon) {
		// no initial value
		return newNode<VarDeclStmt>(st,(*(cur++))->pos().second, _type, id);
	}
	else if(nextToken == Assign){
		cur++;
		auto expr = expression();				// initial value exprssion
		if (expr == nullptr) 
			throw SyntaxError("Parser error: invaild initial value.", (*cur)->pos().first);
		if((*cur)->tag() != Semicolon)
			throw SyntaxError("Parser error: Missing ';'.", (*cur)->pos().first);
		return newNode<VarDeclStmt>(st, (*(cur++))->pos().second, _type, id, expr);
	}
	throw SyntaxError("Parser error: invaild variable declaration.", (*cur)->pos().first);
}

std::shared_ptr<IfStmt> Parser::ifStmt() {
	if ((*cur)->tag() != If) return nullptr;
		
	auto st = (*cur)->pos().first;

	if((*(++cur))->tag() != LeftBracket) 
		throw SyntaxError("Parser error: missing '('.", (*cur)->pos().first);
	cur++;
	auto condition = expression();
	if(condition == nullptr) 
		throw SyntaxError("Parser error: invalid condition.", (*cur)->pos().first);
	if ((*cur)->tag() != LeftBracket)
		throw SyntaxError("Parser error: missing '('.", (*cur)->pos().first);
	cur++;
	auto then = statement();
	if(then == nullptr)
		throw SyntaxError("Parser error: missing statement after 'if'.", (*cur)->pos().first);
	if ((*cur)->tag() != Else)
		return newNode<IfStmt>(st, (*(cur++))->pos().second, condition, then); // ne else
	cur++;
	auto _else = statement();
	if (_else == nullptr)
		throw SyntaxError("Parser error: missing statement after 'else'.", (*cur)->pos().first);
	return newNode<IfStmt>(st, (*(cur++))->pos().second, condition, then, _else); // ne else
}

std::shared_ptr<ForStmt> Parser::forStmt() {
	if ((*cur)->tag() != For) return nullptr;

	auto st = (*cur)->pos().first;
	if ((*(++cur))->tag() != LeftBracket)
		throw SyntaxError("Parser error: missing '('.", (*cur)->pos().first);
	cur++;

	auto init = statement();
	if(init == nullptr) 
		throw SyntaxError("Parser error: missing initial statement after 'for'.", (*cur)->pos().first);
	auto condition = expression();
	if (condition == nullptr)
		throw SyntaxError("Parser error: missing condition after 'for'.", (*cur)->pos().first);
	auto iteration = statement();
	if (iteration == nullptr)
		throw SyntaxError("Parser error: missing iteration after 'for'.", (*cur)->pos().first);

	if ((*(cur++))->tag() != RightBracket)
		throw SyntaxError("Parser error: missing ')'.", (*cur)->pos().first);
	auto body = statement();
	if(body == nullptr)
		throw SyntaxError("Parser error: missing body after 'for'.", (*cur)->pos().first);

	return newNode<ForStmt>(st,body->endPos(),init,condition,iteration,body);
}

std::shared_ptr<WhileStmt> Parser::whileStmt() {
	if ((*cur)->tag() != While) return nullptr;

	auto st = (*cur)->pos().first;
	if ((*(++cur))->tag() != LeftBracket)
		throw SyntaxError("Parser error: missing '('.", (*cur)->pos().first);
	cur++;

	auto condition = expression();
	if (condition == nullptr)
		throw SyntaxError("Parser error: missing condition after 'while'.", (*cur)->pos().first);


	if ((*(cur++))->tag() != RightBracket)
		throw SyntaxError("Parser error: missing ')'.", (*cur)->pos().first);
	auto body = statement();
	if (body == nullptr)
		throw SyntaxError("Parser error: missing body after 'for'.", (*cur)->pos().first);
	return newNode<WhileStmt>(st, body->endPos(), condition, body);
}

std::shared_ptr<ReturnStmt> Parser::returnStmt() {
	if ((*cur)->tag() != Return) return nullptr;

	auto st = (*cur)->pos().first;

	if ((*(++cur))->tag() == Semicolon) {
		// return void type.
		return newNode<ReturnStmt>(st, (*(cur++))->pos().second);
	}

	auto expr = expression();
	if (expr == nullptr)
		throw SyntaxError("Parser error: missing expression after 'return'.", (*cur)->pos().first);

	if((*cur)->tag() != Semicolon)
		throw SyntaxError("Parser error: missing ';'.", (*cur)->pos().first);
	return newNode<ReturnStmt>(st, (*(cur++))->pos().second, expr);
}

std::shared_ptr<BreakStmt> Parser::breakStmt() {
	if ((*cur)->tag() != Break) return nullptr;

	auto st = (*cur)->pos().first;

	if ((*(++cur))->tag() != Semicolon) 
		throw SyntaxError("Parser error: missing ';'.", (*cur)->pos().first);
	return newNode<BreakStmt>(st, (*(cur++))->pos().second);
}

std::shared_ptr<ContinueStmt> Parser::continueStmt() {
	if ((*cur)->tag() != Continue) return nullptr;

	auto st = (*cur)->pos().first;

	if ((*(++cur))->tag() != Semicolon)
		throw SyntaxError("Parser error: missing ';'.", (*cur)->pos().first);
	return newNode<ContinueStmt>(st, (*(cur++))->pos().second);
}

std::shared_ptr<EmptyStmt> Parser::emptyStmt() {
	auto st = (*cur)->pos().first;
	if((*cur)->tag() == Semicolon)
		return newNode<EmptyStmt>(st, (*(cur++))->pos().second);
	return nullptr;
}

std::shared_ptr<StmtBlock> Parser::stmtBlock() {
	auto st = (*cur)->pos().first;
	if ((*cur)->tag() != LeftBrace) return nullptr;
	cur++;
	std::vector<std::shared_ptr<Statement> > stmts;
	while (true) {
		auto stmt = statement();
		if (stmt == nullptr) break;
		stmts.emplace_back(stmt);
	}
	if((*cur)->tag() != RightBrace)
		throw SyntaxError("Parser error: missing '}'.", (*cur)->pos().first);
	return newNode<StmtBlock>(st, (*(cur++))->pos().second, stmts);
}

std::shared_ptr<ExprStmt> Parser::exprStmt() {
	auto st = (*cur)->pos().first;
	auto expr = expression();
	if (expr == nullptr) return nullptr;
	
	if ((*(++cur))->tag() != Semicolon)
		throw SyntaxError("Parser error: missing ';'.", (*cur)->pos().first);
	return newNode<ExprStmt>(st, (*(cur++))->pos().second, expr);
}

std::shared_ptr<Statement> Parser::statement() {
	std::shared_ptr<Statement> stmt;
	stmt = varDeclStmt();
	if (stmt != nullptr) return stmt;
	stmt = ifStmt();
	if (stmt != nullptr) return stmt;
	stmt = exprStmt();
	if (stmt != nullptr) return stmt;
	stmt = whileStmt();
	if (stmt != nullptr) return stmt;
	stmt = forStmt();
	if (stmt != nullptr) return stmt;
	stmt = returnStmt();
	if (stmt != nullptr) return stmt;
	stmt = continueStmt();
	if (stmt != nullptr) return stmt;
	stmt = breakStmt();
	if (stmt != nullptr) return stmt;
	stmt = stmtBlock();
	if (stmt != nullptr) return stmt;
	return emptyStmt();
}

/******************************************Statements end here************************************************/


std::shared_ptr<FunctionDecl> Parser::functionDecl() {
	auto st = (*cur)->pos().first;
	auto iter = cur;
	auto retType = type();
	if (retType == nullptr || (*cur)->tag() != Void){
		cur = iter;
		return nullptr;
	}
	if ((*cur)->tag() == Void) cur++;
	auto name = identifier();
	if (name == nullptr || (*cur)->tag() != LeftBracket) {
		cur = iter;
		return nullptr;
	}
	cur++; // skip '('

	auto args = formalArguments();
	
	if((*cur)->tag() != RightBracket)
		throw SyntaxError("Parser error: missing ')'.", (*cur)->pos().first);
	cur++; // skip ')'

	auto body = stmtBlock();
	if(body == nullptr)
		throw SyntaxError("Parser error: missing function body.", (*cur)->pos().first);

	return newNode<FunctionDecl>(st, body->endPos(),
		retType, name, args, body);
}

std::shared_ptr<ClassDecl> Parser::classDecl() {
	auto st = (*cur)->pos().first;
	if ((*cur)->tag() != Class) return nullptr;

	auto className = identifier();
	if(className == nullptr)
		throw SyntaxError("Parser error: missing class name.", (*cur)->pos().first);
	if((*(++cur))->tag() != LeftBrace) 
		throw SyntaxError("Parser error: missing '{'.", (*cur)->pos().first);
	cur++; // skip '{'

	std::vector<std::shared_ptr<Declaration> > decls;
	while (true) {
		auto decl = declaration();
		if (decl == nullptr) {	// might be constructors
			if ((*cur)->tag() != ID || (*cur)->toString() != className->name) return nullptr;
			auto POS = (*cur)->pos();
			if ((*(++cur))->tag() != LeftBracket) return nullptr;
			cur++; // skip '('
			// constructors has no arguments
			if ((*cur)->tag() != RightBracket)
				throw SyntaxError("Parser error: missing ')'.", (*cur)->pos().first);
			cur++; // skip ')'

			auto body = stmtBlock();
			if (body == nullptr) return nullptr;

			decl = newNode<FunctionDecl>(st, body->endPos(),
				std::shared_ptr<Type>(nullptr),
				newNode<Identifier>(POS.first, POS.second, className->name + "::ctor"),
				std::vector< std::shared_ptr<VarDeclStmt> >(),
				body
				);
		}
		if (decl == nullptr) break;
		decls.push_back(decl);
	}
	
	if ((*cur)->tag() != RightBrace)
		throw SyntaxError("Parser error: missing '}'.", (*cur)->pos().first);
	if ((*(++cur))->tag() != Semicolon)
		throw SyntaxError("Parser error: missing ';'.", (*cur)->pos().first);

	return newNode<ClassDecl>(st, (*(cur++))->pos().second, className, decls);
}

std::shared_ptr<Declaration> Parser::declaration() {
	std::shared_ptr<Declaration> decl = varDeclStmt();
	if (decl != nullptr) return decl;
	decl = functionDecl();
	if (decl != nullptr) return decl;
	return classDecl();
}
/******************************************Program AST Node************************************************/
std::shared_ptr<ProgramAST> Parser::getAST() {
	auto st = (*cur)->pos().first;
	std::vector<std::shared_ptr<Declaration> > decls;
	while (true) {
		auto decl = declaration();
		if (decl == nullptr) break;
		decls.emplace_back(decl);
	}
	if (decls.empty()) return nullptr;
	return newNode<ProgramAST>(st, decls.back()->endPos(), decls);
}

/************************************************Helpers***************************************************/
std::vector<std::shared_ptr<Expression> > Parser::arguments() {
	auto expr1 = expression();
	std::vector<std::shared_ptr<Expression> > ret;
	if (expr1 == nullptr) return ret;
	ret.push_back(expr1);

	while ((*cur)->tag() == Comma) {
		cur++;
		auto expr = expression();
		if (expr == nullptr)
			throw SyntaxError("Parser error: missing argument.", (*cur)->pos().first);
		ret.push_back(expr);
	}
	return ret;
}

std::vector<std::shared_ptr<VarDeclStmt> > Parser::formalArguments() {
	std::vector<std::shared_ptr<VarDeclStmt> > ret;
	if ((*cur)->tag() == RightBracket) return ret;
	auto first = formalArgument();
	if(first == nullptr)
		throw SyntaxError("Parser error: invalid argument.", (*cur)->pos().first);

	ret.push_back(first);
	while ((*cur)->tag() == Comma) {
		cur++;   // skip ','
		auto arg = formalArgument();
		ret.push_back(arg);
	}
	return ret;
}

// this fuction will not return <nullptr>.
std::shared_ptr<VarDeclStmt> Parser::formalArgument() {
	auto st = (*cur)->pos().first;

	auto tp = type();
	if(tp == nullptr)
		throw SyntaxError("Parser error: invalid argument.", (*cur)->pos().first);
	auto name = identifier();
	if (name == nullptr)
		throw SyntaxError("Parser error: invalid argument.", (*cur)->pos().first);
	return newNode<VarDeclStmt>(st, name->endPos(), tp, name);
}
