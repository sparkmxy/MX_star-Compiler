#include "pch.h"
#include "lexer.h"

int main() {
	std::ifstream fin("code.txt");
	char ch;
	Lexer lexer(fin);
	auto V = lexer.getTokens();
	for (auto &token : V)
		std::cout<<token->toString()<<std::endl;
	std::cin >> ch;
	return 0;
}