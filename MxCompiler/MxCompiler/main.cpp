#define _CRT_SECURE_NO_WARNINGS

#include "pch.h"
#include "test.h"

int main() {
	freopen("log.txt", "w", stderr);
	Test test(true);
	test.runWithInterpreter("sha_1.mx");
	system("pause");
	return 0;
}