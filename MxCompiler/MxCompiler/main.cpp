#define _CRT_SECURE_NO_WARNINGS

#include "pch.h"
#include "test.h"

int main() {
	freopen("log.txt", "w", stderr);
	Test test;
	test.test1();
	test.test2();
	//test.runTestCase("codegen/t65.mx");
	system("pause");
	return 0;
}