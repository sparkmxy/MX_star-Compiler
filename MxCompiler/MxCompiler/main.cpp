#define _CRT_SECURE_NO_WARNINGS

#include "pch.h"
#include "test.h"

int main() {
	freopen("log.txt", "w", stderr);
	Test test;
	test.runWithInterpreter("sema/misc-package/misc-23.mx");
	system("pause");
	return 0;
}