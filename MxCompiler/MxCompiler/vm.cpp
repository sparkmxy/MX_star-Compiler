#include "vm.h"

int MemoryManager::getRegValue(const std::string & reg)
{
	return 0;
}

void MemoryManager::setRegValue(const std::string & reg, int v)
{
}

int MemoryManager::load(int addr, const std::string &reg)
{

	return 0;
}

void MemoryManager::store(int addr, const std::string &reg)
{
}

int MemoryManager::allocate_memory(int size)
{
	return 0;
}

void MemoryManager::doubleSpace()
{
	char *newMem = new char[poolSize << 1];
	for (int i = 0; i < used; i++)
		newMem[i] = mem[i];
	delete[] mem;
	mem = newMem;
	poolSize <<= 1;
}
