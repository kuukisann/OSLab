#pragma once

#include <vector>
#include <fstream>
#include "Log.h"

using namespace std;

class MemoryPool
{
private:
	string swapFilename = "osSwap.swap";

protected:
	//size of mem will malloc from real memory (Byte)
	int physicalMemSize;
	//size of swap file (Byte)
	int swapFileSize;

	//the physical mem ptr
	void *physicalPtr;
	//the swap file ptr
	fstream swapStream;

public:
	MemoryPool(int physicalMemSize, int swapFileSize);
	~MemoryPool();

	/*
	//return value: the vector contain the index of the mem block
	//if mem not enough, it will return a empty vector
	*/
	virtual vector<int> osMalloc(int memSize) = 0;

	/*
	//arg: the index of block mem need to free
	//return value: if free success
	*/
	virtual bool osFree(vector<int> memBlock) = 0;

	/*
	//arg: dst: destination of mem to be copied
	//src: source of mem copied
	//num: number of byte to be copied
	//return value: if suc
	*/
	//virtual bool osMemcpy(int dst, const int src, int num) = 0;

	/*
	//arg: the memIndex need to use
	*/
	virtual void osActive(int memIndex) = 0;
};

