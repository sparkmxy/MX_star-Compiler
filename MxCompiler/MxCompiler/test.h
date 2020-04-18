#pragma once

#include "pch.h"
#include "MxCompiler.h"
#include "interpreter.h"

#include<fstream>

class Test {
public:
	void test1();
	bool runTestCase(std::string src);
	void runAndPrintIRCode(std::string src);
	void runWithInterpreter(std::string src);

private:

	bool compile(std::string src);

	void drawRule(std::string str = "");

	void congratulations();
};