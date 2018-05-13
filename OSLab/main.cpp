#include "console\Console.h"
#include "log\Log.h"

int main(int argc, char** args)
{
	Console con;
	while (!con.isExit)
	{
		con.waitForInput();
	}
	fclose(Log::fp);
	return 0;
}