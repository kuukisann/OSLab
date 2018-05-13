#include "PageMemoryPool.h"
#include "../log/Log.h"

#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>

using namespace std;

int PageMemoryPool::getReplacedPageIndexInMem()
{
	while (clockArray[clockIndex] != 0)
	{
		if (clockArray[clockIndex] == 1) {
			clockArray[clockIndex] = 0;
		}
		clockIndex = (clockIndex + 1) % nMemPage;
	}
	return clockIndex;
}

bool PageMemoryPool::replaceMemPage(int nNeededFreePage)
{
	for (int i = 0; i < nNeededFreePage; i++)
	{
		int replacedMemPageIndex = getReplacedPageIndexInMem();
		auto replacedVituralPageIter = find(pageTable.begin(), pageTable.end(), replacedMemPageIndex);
		if (replacedVituralPageIter != pageTable.end())
		{
			int tmpSwapPageIndex = freeSwapPage.back();
			freeSwapPage.pop_back();

			//put page into swap file
			void * tmpBlock = malloc(pageSize);
			getBlockContentWithRealIndex(replacedMemPageIndex, tmpBlock);
			setBlockContentWithRealIndex(tmpSwapPageIndex, tmpBlock);
			free(tmpBlock);

			//put page into free mem page list
			freeMemPage.push_back(replacedMemPageIndex);

			//set clockArray free
			clockArray[replacedMemPageIndex] = -1;

			//change virtual mem index
			*replacedVituralPageIter = tmpSwapPageIndex;

			Log::d("Page %d (in Block %d) swap to Block %d\n", replacedVituralPageIter - pageTable.begin(), replacedMemPageIndex, tmpSwapPageIndex);
		}
		else
			return false;
	}
	return true;
}

PageMemoryPool::PageMemoryPool(int physicalMemSize, int swapFileSize, int pageSize = 0)
	:MemoryPool(physicalMemSize, swapFileSize), pageSize(pageSize == 0 ? 4096 : pageSize)
{
	nMemPage = physicalMemSize / pageSize;
	nSwapPage = swapFileSize / pageSize;
	pageTable.clear();

	for (int i = 0; i < nMemPage + nSwapPage; i++) {
		freeVirtualPage.push_back(i);
		pageTable.push_back(-1);
	}

	for (int i = 0; i < nMemPage; i++) {
		freeMemPage.push_back(i);
		clockArray.push_back(-1);
	}
	for (int i = nMemPage; i < nMemPage + nSwapPage; i++)
		freeSwapPage.push_back(i);
}


PageMemoryPool::~PageMemoryPool()
{
}


vector<int> PageMemoryPool::osMalloc(int memSize)
{
	vector<int> result;
	try
	{
		int nPageNeeded = int(ceil(memSize / pageSize));
		if (nPageNeeded > nMemPage)
		{

			Log::w("Malloced a memSize > physicalMemSize\n");
			return result;
		}
		int nFreeMemPage = freeMemPage.size();
		int nFreeSwapPage = freeSwapPage.size();
		Log::d("Malloc %d B as %d page", memSize, nPageNeeded);
		if (nFreeMemPage + nFreeSwapPage < nPageNeeded)
		{
			Log::i("No enough space even in Swap file\n");
			return result;
		}
		if (freeMemPage.size() < nPageNeeded)
		{
			//swap
			if (!replaceMemPage(nPageNeeded - (freeMemPage.size())))
				Log::w("replace mem page error\n");
		}
		for (int i = 0; i < nPageNeeded; i++)
		{
			//put real index into page table
			int tmpMemPageIndex = freeMemPage.back();
			int tmpVirtualPageIndex = freeVirtualPage.back();
			freeMemPage.pop_back();
			freeVirtualPage.pop_back();

			//set clockArray used recently
			clockArray[tmpMemPageIndex] = 1;

			//put real index into pageTable
			pageTable[tmpVirtualPageIndex] = tmpMemPageIndex;
			result.push_back(tmpVirtualPageIndex);
		}
		Log::i("malloc success, size of vector is %d\n", result.size());
		return result;
	}
	catch (const std::exception& e)
	{
		Log::w("%s\n", e.what());
		return result;
	}
}

bool PageMemoryPool::osFree(vector<int> memBlock)
{
	while (memBlock.size() > 0)
	{
		int tmpVirtualIndex = memBlock.back();
		memBlock.pop_back();

		int tmpRealIndex = pageTable[tmpVirtualIndex];
		Log::d("page %d is being freed, the real index is %d\n", tmpVirtualIndex, tmpRealIndex);

		//free pageTable
		pageTable[tmpVirtualIndex] = -1;
		freeVirtualPage.push_back(tmpVirtualIndex);

		if (tmpRealIndex < nMemPage)
		{
			clockArray[tmpRealIndex] = -1;
			freeMemPage.push_back(tmpRealIndex);
		}
		else
		{
			freeSwapPage.push_back(tmpRealIndex);
		}
	}
	return true;
}

bool PageMemoryPool::getBlockContentWithRealIndex(const int realIndex, void * dst)
{
	if (realIndex < nMemPage)
	{
		char * memBlockPtr = (char *)physicalPtr + (pageSize*realIndex);
		memcpy(dst, memBlockPtr, pageSize);
	}
	else
	{
		swapStream.seekg((realIndex - nMemPage)*pageSize, ios_base::beg);
		swapStream.read((char *)dst, pageSize);
	}
	return true;
}

bool PageMemoryPool::setBlockContentWithRealIndex(const int realIndex, const void * src)
{
	if (realIndex < nMemPage)
	{
		char * memBlockPtr = (char *)physicalPtr + (pageSize*realIndex);
		memcpy(memBlockPtr, src, pageSize);
	}
	else
	{
		swapStream.seekp((realIndex - nMemPage)*pageSize, ios_base::beg);
		swapStream.write((char *)src, pageSize);
	}
	return true;
}

bool PageMemoryPool::getBlockContent(const int blockIndex, void * dst)
{
	int tmpRealIndex = pageTable[blockIndex];
	if (tmpRealIndex == -1)
		return false;
	return getBlockContentWithRealIndex(tmpRealIndex, dst);
}

bool PageMemoryPool::setBlockContent(const int blockIndex, const void * src)
{
	int tmpRealIndex = pageTable[blockIndex];
	if (tmpRealIndex == -1)
		return false;
	return setBlockContentWithRealIndex(tmpRealIndex, src);
}

void PageMemoryPool::osActive(int memBlockIndex)
{
	Log::d("mem page %d is actived", memBlockIndex);
	int tmpRealIndex = pageTable[memBlockIndex];
	if (tmpRealIndex == -1)
	{
		Log::w("real Index out of range\n");
		return;
	}
	if (tmpRealIndex < nMemPage)
	{
		Log::d("needn't swap\n");
		return;
	}

	if (freeMemPage.size() <= 0)
	{
		replaceMemPage(1);
	}

	int tmpMemPageIndex = freeMemPage.back();
	freeMemPage.pop_back();

	//put page into swap file
	void * tmpBlock = malloc(pageSize);
	getBlockContentWithRealIndex(tmpRealIndex, tmpBlock);
	setBlockContentWithRealIndex(tmpMemPageIndex, tmpBlock);
	free(tmpBlock);

	//put page into free mem page list
	freeSwapPage.push_back(tmpRealIndex);

	//set clockArray free
	clockArray[tmpMemPageIndex] = 1;

	//change virtual mem index
	pageTable[memBlockIndex] = tmpMemPageIndex;
	Log::d("swap successfully\n");
}

PageMemStatus PageMemoryPool::getMemStatus()
{
	PageMemStatus tmpStatus;
	tmpStatus.nPhysicalPage = nMemPage;
	tmpStatus.nSwapPage = nSwapPage;
	tmpStatus.nFreePhysicalPage = freeMemPage.size();
	tmpStatus.nFreeSwapPage = freeSwapPage.size();
	return tmpStatus;
}

void PageMemoryPool::saveMemToFile()
{
	ofstream memFile("memory.tmp");
	memFile.write((char*)physicalPtr, physicalMemSize);
	memFile.close();
}


