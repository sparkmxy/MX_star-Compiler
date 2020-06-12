#include "MxCompiler.h"

//#define SHOW_TOKENS

void MxCompiler::compile(bool opt)
{
	getCode();
	parse();
	buildAST();
	semanticCheck();
	generateIR();
	if(opt) optimize();

	std::ofstream fout("ir.mxx");
	printIR(fout);
	fout.close();

	codegen();
}

void MxCompiler::semantic()
{
	getCode();
	parse();
	buildAST();
	semanticCheck();
}

void MxCompiler::getCode()
{
	src.open(fileName);
	if (!src) 
		throw Error("file not found");
	std::cout << "File found." << std::endl;
	std::clog << "--------------------Open: " << fileName << "-------------------\n";
}

void MxCompiler::parse()
{
	lexer = std::make_shared<Lexer>(src);
	tokens = lexer->getTokens();
	src.close();
	
#ifdef  SHOW_TOKENS			//print tokens to log.txt
	std::clog << "------------------------Tokens---------------------\n";
	for (auto &token : tokens)
		std::clog << token->toString() << '\n';
	std::clog << "---------------------------------------------------\n";
#endif //  SHOW_TOKENS

	
	std::cout << "Parse done!" << std::endl;
}

void MxCompiler::buildAST()
{
	parser = std::make_shared<Parser>(tokens);
	ast = parser->getAST();
	if (!parser->finished()) 
		throw Error("redundant tokens.");
	std::cout << "Build AST successfully!" << std::endl;
}

void MxCompiler::semanticCheck()
{
	environment = std::make_shared<Environment>(ast);
	
	environment->semanticCheck();
	std::cout << "Passed semantic check!" << std::endl;
}

void MxCompiler::printIR(std::ostream &os)
{
	std::make_shared<IR_Printer>(ir, os)->print();
}

void MxCompiler::generateIR()
{
	irGenerator = std::make_shared<IR_Generator>(environment->globalScope(),ast);
	irGenerator->generate();
	ir = irGenerator->getIR();

	//printIR(std::cout);
	//std::make_shared<FunctionInliner>(ir)->run();
	//printIR(std::cerr);
	std::make_shared<GlobalVarResolver>(ir)->run();

	std::cout << "IR generation completed." << std::endl;

}

void MxCompiler::optimize()
{
	opt = std::make_shared<Optimizer>(ir);
	opt->optimize();
	// std::make_shared<IR_Printer>(ir, std::cout)->print();
}

void MxCompiler::codegen()
{
	std::ofstream asmfile("test.s");
	codeGenerator = std::make_shared<RISCVCodeGenerator>(ir, asmfile);
	codeGenerator->generate();
	codeGenerator->emit();
	asmfile.close();
}
