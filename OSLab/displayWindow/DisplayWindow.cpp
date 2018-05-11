#include "DisplayWindow.h"
#include "../log/Log.h"
#include "../filesystem/disk.h"
#include <string>
#include <vector>

using namespace std;

struct PCB_Show
{
	unsigned int pid; //进程PID
	string name; // 进程名称
	int size; //进程占用内存块大小
	enum procstate { NEW, READY, RUN, WAITING, FINISH } state;  //进程状态
	unsigned int prio;//优先级
	int serviceTime;//进程服务时间
	int runTime;//已经占用CPU时间
};

struct PageMemStatus {
	int nPhysicalPage;
	int nFreePhysicalPage;
	int nSwapPage;
	int nFreeSwapPage;
};


DisplayWindow::DisplayWindow(ProcSys *procSys, PageMemoryPool *memoryPool) :procSys(procSys), memPool(memoryPool)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();
	window = SDL_CreateWindow("status", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		900, 900, SDL_WINDOW_SHOWN);
	windowSurface = SDL_GetWindowSurface(window);
	font = TTF_OpenFont("src/Roboto-Regular.ttf", 20);
	if (!font)
	{
		//Log::w("font open error\n");
	}
}


DisplayWindow::~DisplayWindow()
{
}


void DisplayWindow::refreshWindow()
{
	//clear window
	SDL_FillRect(windowSurface, NULL, SDL_MapRGB(windowSurface->format, 0, 0, 0));

	//refresh ps

	//todo: get message with ps
	SDL_Rect psRect = { 30,30,840,540 };
	vector<PCB_Show> processList = vector<PCB_Show>();
	for (int i = 0; i < processList.size() && i < 18; i++)
	{
		PCB_Show* tmppcb = &processList[i];
		string pid = to_string(tmppcb->pid);
		string name = tmppcb->name;
		string size = to_string(tmppcb->size);
		string state = to_string(tmppcb->state);
		string prio = to_string(tmppcb->prio);
		string serviceTime = to_string(tmppcb->serviceTime);
		string runTime = to_string(tmppcb->runTime);

		string tmpLIst = "";
		tmpLIst += pid;
		tmpLIst += string(10 - pid.length(), ' ');
		tmpLIst += name;
		tmpLIst += string(30 - name.length(), ' ');
		tmpLIst += size;
		tmpLIst += string(20 - size.length(), ' ');
		tmpLIst += state;
		tmpLIst += string(5 - state.length(), ' ');
		tmpLIst += prio;
		tmpLIst += string(10 - prio.length(), ' ');
		tmpLIst += serviceTime;
		tmpLIst += string(10 - serviceTime.length(), ' ');
		tmpLIst += runTime;
		tmpLIst += string(10 - runTime.length(), ' ');
		SDL_Surface* textLine = TTF_RenderText_Solid(font, tmpLIst.c_str(), SDL_Color{ 255,255,255 });
		SDL_BlitSurface(textLine, NULL, windowSurface, &psRect);
		psRect.y += 30;
	}


	//refresh mem status

	//todo: get message with mem
	SDL_Rect memRect = { 30,630,240,240 };
	PageMemStatus memStatus = memPool->getMemStatus();
	string tmpStr = "nPhysicalPage: " + to_string(memStatus.nPhysicalPage);
	SDL_Surface* textLine = TTF_RenderText_Solid(font, tmpStr.c_str(), SDL_Color{ 255,255,255 });
	SDL_BlitSurface(textLine, NULL, windowSurface, &memRect);
	memRect.y += 30;
	tmpStr = "nSwapPage: " + to_string(memStatus.nSwapPage);
	SDL_Surface* textLine = TTF_RenderText_Solid(font, tmpStr.c_str(), SDL_Color{ 255,255,255 });
	SDL_BlitSurface(textLine, NULL, windowSurface, &memRect);
	memRect.y += 30;
	tmpStr = "nFreePhysicalPage: " + to_string(memStatus.nFreePhysicalPage);
	SDL_Surface* textLine = TTF_RenderText_Solid(font, tmpStr.c_str(), SDL_Color{ 255,255,255 });
	SDL_BlitSurface(textLine, NULL, windowSurface, &memRect);
	memRect.y += 30;
	tmpStr = "nFreeSwapPage: " + to_string(memStatus.nFreeSwapPage);
	SDL_Surface* textLine = TTF_RenderText_Solid(font, tmpStr.c_str(), SDL_Color{ 255,255,255 });
	SDL_BlitSurface(textLine, NULL, windowSurface, &memRect);
	memRect.y += 30;

	//refresh file list

	//todo: get message with ls
	SDL_Rect fileRect = { 330,630,540,240 };
	vector<string> fileList = os_ls();
	for (int i = 0; i < fileList.size() && i < 8; i++)
	{
		SDL_Surface* textLine = TTF_RenderText_Solid(font, fileList[i].c_str(), SDL_Color{ 255,255,255 });
		SDL_BlitSurface(textLine, NULL, windowSurface, &fileRect);
		fileRect.y += 30;
	}

	SDL_UpdateWindowSurface(window);
}