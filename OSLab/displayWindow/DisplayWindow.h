#pragma once
#include <SDL.h>
#include <SDL_ttf.h>

class DisplayWindow
{
private:
	SDL_Window* window;
	SDL_Surface* windowSurface;
	TTF_Font* font;
	FileSystem *fs;
	PageMemoryPool *memoryPool;
public:
	DisplayWindow(FileSystem *fs, PageMemoryPool *memoryPool);
	~DisplayWindow();
	void refreshWindow();
};

