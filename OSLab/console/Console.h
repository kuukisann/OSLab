#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include "PageMemoryPool.h"

using namespace std;

class Console
{
private:
	void Console::InputAnalyse(vector<string> cuttedString);
	string trim(string str);
	void InputCut(string input);
	string workingDir = "/root/";
	PageMemoryPool mempool = PageMemoryPool(10240, 20480, 32);
	FileSystem fs = FileSystem(mempool);
	bool cat(string filename);
	bool rmfile(string filename);
	bool rmdir(string filename);
	bool pwd(string newDir);
	
public:
	bool isExit = false;

	Console();
	~Console();
	void waitForInput();
};

