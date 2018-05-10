#pragma once
#include <ctime>
#include <mutex>
#include <cstdio>

using namespace std;

class Log
{
private:
	static mutex logMutex;
	static FILE* fp;
public:
	static bool ifDebug;

	Log();
	~Log();
	

	/*
	//use to print debug info
	*/
	static bool d(const char * format, ...);

	/*
	//use to print normal info
	*/
	static bool i(const char * format, ...);

	/*
	//use to print warning
	*/
	static bool w(const char * format, ...);
};

