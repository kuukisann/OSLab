#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include "../memory/PageMemoryPool.h"
#include "../ProcManage/PM.h"

#include <thread>

class DisplayWindow
{
private:
	SDL_Window* window;
	SDL_Surface* windowSurface;
	TTF_Font* font;
	PM* procM;
	PageMemoryPool *memPool;
public:
	DisplayWindow(PM *procM, PageMemoryPool *memoryPool);
	~DisplayWindow();
	void refreshWindow();
	void refreshThread();
};

