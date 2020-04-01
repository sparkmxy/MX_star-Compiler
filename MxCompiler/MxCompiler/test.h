#pragma once

#include "pch.h"
#include "MxCompiler.h"

class Test {
public:
	void test1();
	void test2();
	bool runTestCase(std::string src);
private:

	bool complie(std::string src);

	void drawRule(std::string str = "");

	void congratulations();
};