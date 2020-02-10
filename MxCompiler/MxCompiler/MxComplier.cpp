#include "MxComplier.h"

void MxComplier::getCode()
{
	src.open(fileName);
	if (!src) 
		throw Error("file not found",Position());
	std::cout << "File found!" << std::endl;
}

void MxComplier::parse()
{
	lexer = std::make_shared<Lexer>(src);
	tokens = lexer->getTokens();

	//print tokens to log.txt
	std::clog << "----------------------------------------------\n";
	for (auto &token : tokens)
		std::clog << token->toString() << '\n';
	std::clog << "----------------------------------------------\n";

	std::cout << "Parse done!" << std::endl;
}

void MxComplier::buildAST()
{
	parser = std::make_shared<Parser>(tokens);
	ast = parser->getAST();
	if (!parser->finished()) 
		throw Error("redundant tokens.", Position());
	std::cout << "Build AST successfully!" << std::endl;
}

void MxComplier::semanticCheck()
{
	environment = std::make_shared<Environment>(ast);
	
	environment->semanticCheck();
	std::cout << "Passed semantic check!" << std::endl;
}
