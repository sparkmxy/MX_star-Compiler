#include "MxComplier.h"

void MxComplier::getCode()
{
	src.open(fileName);
	if (!src) {
		std::cout << "Error: file not found." << std::endl;
		std::terminate();
	}
	std::cout << "File found!" << std::endl;
}

void MxComplier::parse()
{
	lexer = std::make_shared<Lexer>(src);
	tokens = lexer->getTokens();
	std::cout << "Parse done!" << std::endl;
}

void MxComplier::buildAST()
{
	parser = std::make_shared<Parser>(tokens);
	ast = parser->getAST();
	if (!parser->finished()) {
		std::cout << "Error: redundant tokens." << std::endl;
		std::terminate();
	}
	std::cout << "Build AST successfully!" << std::endl;
}

void MxComplier::semanticCheck()
{
	environment = std::make_shared<Environment>(ast);
	environment->semanticCheck();
	std::cout << "Passed emantic check!" << std::endl;
}
