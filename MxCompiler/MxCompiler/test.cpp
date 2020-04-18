#include "test.h"
#include <Windows.h>

void Test::test1()
{
	for (int i = 1; i <= 9; i++) {
		auto src = "sema/breakcontinue-package/breakcontinue-" + std::to_string(i) + ".mx";
		if (!runTestCase(src)) return;
	}
	congratulations();
}

bool Test::runTestCase(std::string src)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
	drawRule(src);
	bool ok = compile("test/" + src);
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

void Test::runAndPrintIRCode(std::string src)
{
	MxCompiler compiler("test/" + src);
	try
	{
		compiler.complie();
	}
	catch (Error & err)
	{
		std::cout << err.what() << std::endl;
		std::clog << err.what() << std::endl;
		return;
	}
	compiler.printIR();
	std::clog << "FINISHED\n";
}

void Test::runWithInterpreter(std::string src)
{
	MxCompiler compiler("test/" + src);
	try
	{
		compiler.complie();
	}
	catch (Error & err)
	{
		std::cout << err.what() << std::endl;
		std::clog << err.what() << std::endl;
		return;
	}
	std::cout << "COMPILE FINISHED\n";
	std::ofstream fout("ir.mxx");
	compiler.printIR(fout);
	fout.close();
	std::ifstream fin("ir.mxx");
	IR_Interpreter I(fin);
	I.run();
}

bool Test::compile(std::string src) {
	MxCompiler compiler(src);
	try
	{
		compiler.complie();
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
