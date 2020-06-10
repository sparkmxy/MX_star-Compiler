#define _CRT_SECURE_NO_WARNINGS

#include "pch.h"
#include "test.h"

int main() {
	freopen("log.txt", "w", stderr);
	Test test(false);
	test.runWithInterpreter("codegen/t55.mx");
	system("pause");
	return 0;
}