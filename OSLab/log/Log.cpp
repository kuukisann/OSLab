#include "Log.h"

#include <cstdio>
#include <cstdarg>
#include <cstdio>


using namespace std;


bool Log::ifDebug = false;
mutex Log::logMutex;
FILE* Log::fp = fopen("osLog.log", "a");

Log::Log()
{
}


Log::~Log()
{
}

bool Log::d(const char * format, ...)
{
	if (ifDebug)
	{
		logMutex.lock();
		fprintf(fp, "[%ld][Debug]", clock());
		va_list args;
		va_start(args, format);
		vfprintf(fp, format, args);
		va_end(args);
		logMutex.unlock();
	}
	return true;
}

bool Log::i(const char * format, ...)
{
	logMutex.lock();
	fprintf(fp, "[%ld][Info]", clock());
	va_list args;
	va_start(args, format);
	vfprintf(fp, format, args);
	va_end(args);
	logMutex.unlock();
	return true;
}

bool Log::w(const char * format, ...)
{
	logMutex.lock();
	fprintf(fp, "[%ld][Warning]", clock());
	va_list args;
	va_start(args, format);
	vfprintf(fp, format, args);
	va_end(args);
	logMutex.unlock();
	return true;
}

