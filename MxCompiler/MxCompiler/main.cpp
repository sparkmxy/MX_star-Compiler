#define _CRT_SECURE_NO_WARNINGS

#include "pch.h"
#include "test.h"

int main() {
	freopen("log.txt", "w", stderr);
	Test test;
	//test.test1();
	//test.test2();
	test.runTestCase("sema/scope-package/scope-2.mx");
	system("pause");
	return 0;
}