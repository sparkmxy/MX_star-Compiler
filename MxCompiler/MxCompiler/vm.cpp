#include "vm.h"

int MemoryManager::getRegValue(const std::string & name)
{
	return regs[name];
}

void MemoryManager::setRegValue(const std::string & name, int v)
{
	regs[name] = v;
}

int MemoryManager::load(int addr)
{
	return *reinterpret_cast<int *>(addr);
}

void MemoryManager::store(int addr, int v)
{
	*reinterpret_cast<int *>(addr) = v;
}

Byte MemoryManager::loadByte(int addr)
{
	return *reinterpret_cast<Byte *>(addr);
}

Byte *MemoryManager::allocate_memory(int size)
{
	if (used + size > poolSize) {
		while (poolSize < size) poolSize <<= 1;
		mem = new Byte[poolSize];
		memPools.push_back(mem);
		used = 0;
	}
	Byte *ret = mem + used;
	used += size;
	return ret;
}


