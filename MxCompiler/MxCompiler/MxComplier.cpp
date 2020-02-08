#include "MxComplier.h"

void MxComplier::getCode()
{
	src.open(fileName);
	if (!src) {
		std::cout << "Error: file not found." << std::endl;
		std::terminate();
	}
}

void MxComplier::parse()
{
	lexer = std::make_shared<Lexer>(src);
	tokens = lexer->getTokens();
}

void MxComplier::buildAST()
{
	parser = std::make_shared<Parser>(tokens);
	ast = parser->getAST();
	if (!parser->finished()) {
		std::cout << "Error: redundant tokens." << std::endl;
		std::terminate();
	}
}

void MxComplier::semanticCheck()
{
	environment = std::make_shared<Environment>(ast);
	environment->semanticCheck();
}
