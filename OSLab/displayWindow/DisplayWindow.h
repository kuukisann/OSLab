#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include "../memory/PageMemoryPool.h"

class DisplayWindow
{
private:
	SDL_Window* window;
	SDL_Surface* windowSurface;
	TTF_Font* font;
	ProcSys *procSys;
	PageMemoryPool *memPool;
public:
	DisplayWindow(ProcSys *procSys, PageMemoryPool *memoryPool);
	~DisplayWindow();
	void refreshWindow();
};

