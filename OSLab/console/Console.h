#pragma once

#include "../memory/PageMemoryPool.h"
#include "../filesystem/disk.h"

#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>

using namespace std;

class Console
{
private:
	void Console::InputAnalyse(vector<string> cuttedString);
	string trim(string str);
	void InputCut(string input);
	string workingDir = "/root";
	PageMemoryPool memPool;
	//todo
	ProcessSystem procSys;
	bool cat(string filename);
	bool ls();
	bool mkfile(string filename);
	bool mkexec(string filename);
public:
	bool isExit = false;

	Console();
	~Console();
	void waitForInput();
};

