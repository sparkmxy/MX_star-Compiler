#define _CRT_SECURE_NO_WARNINGS

#include "pch.h"
#include "test.h"

int main() {
	freopen("log.txt", "w", stderr);
	Test test;
	test.runAndPrintIRCode("sema/basic-package/basic-3.mx");
	system("pause");
	return 0;
}