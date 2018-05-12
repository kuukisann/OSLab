#include "console\Console.h"
#include <Windows.h>

int main(int argc, char** args)
{
	Console con;
	while (!con.isExit)
	{
		con.waitForInput();
	}
	Sleep(1000);
	return 0;
}