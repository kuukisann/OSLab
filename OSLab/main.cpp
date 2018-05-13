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

	con.disWin.closeWindow();
	fclose(Log::fp);

	Sleep(2000);

	return 0;
}