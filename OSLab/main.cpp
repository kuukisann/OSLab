#include "console\Console.h"

int main(int argc, char** args)
{
	Console con;
	while (!con.isExit)
	{
		con.waitForInput();
	}
	return 0;
}