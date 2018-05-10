#include "MemoryPool.h"


MemoryPool::MemoryPool(int physicalMemSize,int swapFileSize)
	:physicalMemSize(physicalMemSize), swapFileSize(swapFileSize)
{
	physicalPtr = malloc(physicalMemSize);
	swapStream.open(swapFilename, fstream::in | fstream::out);
	if (!swapStream.is_open())
	{
		Log::i("create swap file\n");
		swapStream.open(swapFilename, fstream::out);
		swapStream.close();
		swapStream.open(swapFilename, fstream::in | fstream::out);
		swapStream.clear();
	}
}


MemoryPool::~MemoryPool()
{
	free(physicalPtr);
	swapStream.close();
}
