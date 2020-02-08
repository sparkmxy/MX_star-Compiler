#include "pch.h"
#include "MxComplier.h"

void complie() {
	MxComplier complier("code.txt");
	complier.getCode();
	complier.parse();
	complier.buildAST();
	complier.semanticCheck();
}

int main() {
	try
	{
		complie();
	}
	catch (Error & err)
	{
		std::cout << "complie failed.\n" << err.what() << std::endl;
	}
	std::cout << "Congratulations! Complie successfully." << std::endl;
	return 0;
}