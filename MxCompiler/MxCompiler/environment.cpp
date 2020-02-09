#include "environment.h"

void Environment::builtinTypeInit()
{
	//Builtin types : int, bool, void
	global->define(intSymbol);
	global->setIntSymbol(intSymbol);

	global->define(boolSymbol);
	global->setBoolSymbol(boolSymbol);

	global->define(voidSymbol);
	global->setVoidSymbol(voidSymbol);

	//Class string : length(), substring()
	auto length = std::make_shared<FunctionSymbol>("length", intSymbol, nullptr, stringSymbol);
	stringSymbol->define(length);
	auto substring = std::make_shared<FunctionSymbol>("substring", stringSymbol, nullptr, stringSymbol);
	auto arg1 = std::make_shared<VarSymbol>("left", intSymbol, nullptr);
	auto arg2 = std::make_shared<VarSymbol>("right", intSymbol, nullptr);

	substring->define(arg1);
	substring->define(arg2);

	stringSymbol->define(substring);
	global->setStringSymbol(stringSymbol);
}

/*
Builtin Functions:
void print(string)
void println(string)
string getString()
int getInt()
string toString(int)
int array.size()
*/
void Environment::builtinFuncInit()
{
	auto print = std::make_shared<FunctionSymbol>("print", voidSymbol, nullptr, global);
	print->define(std::make_shared<VarSymbol>("str", stringSymbol, nullptr));
	global->define(print);
	
	auto println = std::make_shared<FunctionSymbol>("println", voidSymbol, nullptr, global);
	print->define(std::make_shared<VarSymbol>("str", stringSymbol, nullptr));
	global->define(println);

	auto getString = std::make_shared<FunctionSymbol>("getString", stringSymbol, nullptr, global);
	global->define(getString);

	auto getInt = std::make_shared<FunctionSymbol>("getInt", intSymbol, nullptr, global);
	global->define(getInt);

	auto toString = std::make_shared<FunctionSymbol>("toString", stringSymbol, nullptr, global);
	toString->define(std::make_shared<VarSymbol>("i", intSymbol, nullptr));
	global->define(toString);

	auto arraySize = std::make_shared<FunctionSymbol>("array.size", intSymbol, nullptr, global);
	global->setArrayFuncSymbol(arraySize);
}

void Environment::buildSymbolTable()
{
	symbolTable = std::make_shared<SymbolTable>(global);
	symbolTable->visit(ast.get());
}

void Environment::semanticCheck()
{
	semanticChecker = std::make_shared<SemanticChecker>(global);
	semanticChecker->visit(ast.get());
}


/*
This will define an extra funciton <bootstrap> in the AST.
<bootstrap> simply assign initial value of global variables 
and then call <main>.
*/
void Environment::bootstrapFuncInit()
{
	std::vector<std::shared_ptr<Statement> > stmts;

	// initialization of global variables 
	auto decls = ast->getDecls();
	for (auto &decl : decls) 
		if(decl->isVarDecl()){
			auto initExpr = std::static_pointer_cast<VarDeclStmt>(decl)->getInitExpr();
			auto idExpr = std::make_shared<IdentifierExpr>(
				std::static_pointer_cast<VarDeclStmt>(decl)->getIdentifier());
			if (initExpr != nullptr) {
				auto assignExpr = 
					std::make_shared<BinaryExpr>(BinaryExpr::ASSIGN, idExpr, initExpr);
				auto stmt = std::make_shared<ExprStmt>(assignExpr);
				stmts.emplace_back(stmt);
			}
		}

	// return main();
	auto Main = std::make_shared<IdentifierExpr>(std::make_shared<Identifier>("main"));
	auto callMain = std::make_shared<FuncCallExpr>(
			Main,
			std::vector<std::shared_ptr<Expression> >()		// no args
		);
	auto returnMain = std::make_shared<ReturnStmt>(callMain);
	stmts.emplace_back(returnMain);

	// define bootstrap and put it into AST
	auto bootstrap = std::make_shared<FunctionDecl>(
			nullptr, // stands for void type
			std::make_shared<Identifier>("bootstrap"),
			std::vector< std::shared_ptr<VarDeclStmt> >(), // <bootstrap> has no args
			std::make_shared<StmtBlock>(stmts)
		);
	ast->getDecls().emplace_back(bootstrap);
}
