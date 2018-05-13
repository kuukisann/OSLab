#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include "../memory/PageMemoryPool.h"
#include "../ProcManage/PM.h"


class DisplayWindow
{
private:
	PM* procM;
	PageMemoryPool *memPool;
public:
	SDL_Window* window;
	SDL_Surface* windowSurface;
	TTF_Font* font;
	DisplayWindow(PM *procM, PageMemoryPool *memoryPool);
	~DisplayWindow();
	void refreshWindow();
	bool isExit;
};

