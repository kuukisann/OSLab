#pragma once
#include "MemoryPool.h"


using namespace std;

struct PageMemStatus {
	int nPhysicalPage;
	int nFreePhysicalPage;
	int nSwapPage;
	int nFreeSwapPage;
};

class PageMemoryPool :
	public MemoryPool
{
private:
	//page size is (pageSize B)
	int pageSize;

	int nMemPage;
	int nSwapPage;

	//the index of virtual page unused
	vector<int> freeVirtualPage;
	//the index is the index of virtual memory
	//the vector include the real index of every page put in
	vector<int> pageTable;

	//the vector include the real index of free memory page
	vector<int> freeMemPage;

	//the vector include the index of free swap file page
	vector<int> freeSwapPage;

	//index is the real index of memory page
	//the vector include the state of memory page
	//	1:used recently
	//	0:not used recently
	//	-1:free
	vector <int> clockArray;
	//the index clock algorithm now use
	int clockIndex = 0;


	//get the index need to be replaced in the memory
	int getReplacedPageIndexInMem();

	//put mem page into free swap file page
	bool replaceMemPage(int nNeededFreePage);

	//like getBlockContent but use real index
	bool getBlockContentWithRealIndex(const int blockIndex, void * dst);

	//like setBlockContent but use real index
	bool setBlockContentWithRealIndex(const int blockIndex, const void * src);

public:
	PageMemoryPool(int physicalMemSize, int swapFileSize, int pageSize);
	~PageMemoryPool();

	/*
	//return value: the vector contain the index of the mem block
	//if mem not enough, it will return a empty vector
	//!!!!please let memSize <= physicalMemSize
	*/
	vector<int> osMalloc(int memSize);

	/*
	//arg: the index of block mem need to free
	//return value: if free success
	*/
	bool osFree(vector<int> memBlock);

	/*
	//arg: dst: the page index of the destination mem to be copied
	//src: the page index of mem copied
	//num: number of block to be copied
	//return value: if suc
	*/
	//bool osMemcpy(int dstBlockIndex, const int srcBlockIndex, int num);


	/*
	//arg: 
	//blockIndex: the page index you want to get content
	//dst: the space to put data into (the size of dst is pageSize B)
	//return value: if suc
	*/
	bool getBlockContent(const int blockIndex, void* dst);

	/*
	//arg:
	//blockIndex: the page index you want to set content
	//src: the space have the data you want to put into block (the size of src is pageSize B)
	//return value: if suc
	*/
	bool setBlockContent(const int blockIndex, const void* src);

	/*
	//arg: the memIndex need to use
	*/
	void osActive(int memBlockIndex);

	PageMemStatus getMemStatus();
	void saveMemToFile();
};

