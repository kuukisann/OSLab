#include "Console.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>

using namespace std;


Console::Console()
{

}


Console::~Console()
{
}

string Console::trim(string str)
{
	str.erase(0, str.find_first_not_of(" "));
	str.erase(str.find_last_not_of(" ") + 1);
	return str;
}


void Console::waitForInput()
{
	//TODO: mutex lock
	
	string tmp;
	string currentDir = "/";
	cout << endl << currentDir << "$ ";
	getline(cin, tmp);

	tmp = trim(tmp);

	if (tmp!="")
		InputCut(tmp);

	//TODO: mutex unlock
}


void Console::InputCut(string input)
{
	stringstream stream;

	//put input string into a string stream
	//and then read out split string
	stream << input;
	string tmp;
	vector<string> argv;
	while (!stream.eof())
	{
		stream >> tmp;
		argv.push_back(tmp);
	}

	InputAnalyse(argv);
}

bool Console::cat(string filename)
{
	os_file* fp = Open_File(filename);
	if (!fp)
	{
		cout << "open file error\n" << endl;
		return false;
	}
	int fileSize = get_file_size(fp);
	void* dst = malloc(fileSize+3);
	if (os_fread(dst, fileSize, fp)) 
	{
		dst[fileSize] = 0;
		cout << (char*)dst << endl;
		return true;
	}
	else
	{
		cout << "fread error" << endl;
		return false;
	}
	return false;
}

bool Console::pwd(string newDir)
{
	if (os_cd(workingDir, newDir))
		return true;
	else
		return false;
}

void Console::InputAnalyse(vector<string> args)
{
	int inPos = 0;

	string command = args[0];
	//exit
	if (command == "exit")
		isExit = true;
	else if (command == "rmfile")
	{

	}
	else if (command == "rmdir")
	{

	}
	else if (command == "cd")
	{

	}
	else if (command == "mkdir")
	{

	}
	else if (command == "rm")
	{

	}
	else if (command == "ls")
	{
		 
	}
	else if (command == "exec")
	{

	}
	else if (command == "kill")
	{

	}
	else if (command == "ps")
	{

	}
	else if (command == "cat")
	{
		cat(args[1]);
	}




	else if (command == "ls")
	{

	}
	else if (command == "ls")
	{
	}
	else if (command == "ls")
	{

	}
	else if (command == "ls")
	{


	}

}