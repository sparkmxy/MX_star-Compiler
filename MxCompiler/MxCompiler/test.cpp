#include "test.h"
#include <Windows.h>

void Test::test1()
{
	for (int i = 1; i <= 68; i++) {
		auto src = "codegen/t" + std::to_string(i) + ".mx";
		if (!runTestCase(src)) return;
	}
	congratulations();
}

void Test::test2()
{
	for (int i = 1; i <= 10; i++) {
		auto src = "codegen/e" + std::to_string(i) + ".mx";
		if (!runTestCase(src)) return;
	}
	congratulations();
}

bool Test::runTestCase(std::string src)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
	drawRule(src);
	bool ok = complie("test/" + src);
	if (!ok) {
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
		std::cout << "FAIL" << std::endl;
	}
	else {
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
		std::cout << "PASS" << std::endl;
	}
	return ok;
}

bool Test::complie(std::string src) {
	MxCompiler complier(src);
	try
	{
		complier.getCode();
		complier.parse();
		complier.buildAST();
		complier.semanticCheck();
	}
	catch (Error & err)
	{
		std::cout << err.what() << std::endl;
		std::clog << err.what() << std::endl;
		return false;
	}
	std::clog << "FINISHED\n";
	return true;
}

void Test::drawRule(std::string str)
{
	std::cout << "--------------------" << str << "--------------------" << std::endl;
}

void Test::congratulations()
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
	std::cout << "Congratulations! All testcases passed." << std::endl;
	Sleep(1000);
}
