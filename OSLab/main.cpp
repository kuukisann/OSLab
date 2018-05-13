#include "console\Console.h"
#include "log\Log.h"
#include <Windows.h>

int main(int argc, char** args)
{
	Console con;

	SDL_Event event;
	int preClock = clock();
	while (!con.disWin.isExit)
	{
		SDL_PollEvent(&event);
		if (clock() - preClock >= 500)
		{
			preClock = clock();
			con.disWin.refreshWindow();
		}
	}

	fclose(Log::fp);
	con.disWin.closeWindow();

	return 0;
}