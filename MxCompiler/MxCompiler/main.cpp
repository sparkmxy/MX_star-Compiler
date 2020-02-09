#define _CRT_SECURE_NO_WARNINGS

#include "pch.h"
#include "MxComplier.h"

void complie(std::string src) {
	MxComplier complier(src);
	try
	{
		complier.getCode();
		complier.parse();
		complier.buildAST();
		complier.semanticCheck();
	}
	catch (Error & err)
	{
		std::clog << "FAILED\n";
		std::cout << "Unfortunately, complie has failed.\n" << err.what() << std::endl;
		return;
	}
	std::clog << "FINISHED\n";
	std::cout << "Congratulations! Complie successfully." << std::endl;
}

int main() {
	freopen("log.txt", "w", stderr);
	
	std::string src = "code.txt";

	complie(src);
	
	system("pause");
	return 0;
}