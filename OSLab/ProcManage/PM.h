#pragma once

#include<list>
#include<vector>
#include"PCB.h"

using namespace std;



#define MAXMEMORYSIZE 10240//物理内存(Bytes)  10MB
#define PAGESIZE 32//页大小(Bytes) 
#define MAXSWAPFILESIZE 20480 //最大分页文件大小 20MB

#define MAXPROC 64 //最大进程数 readlist和waitlist的总进程数

#define MAXPAGENUM 320 //总页数

#define MAXPROCMEM 160 // 单个进程最大占用内存 160Bytes 目前没用到


struct PCB_Show
{
	unsigned int PID; //进程PID
	string Name; // 进程名称
	int Size; //进程占用内存块大小
	procstate State;  //进程状态
	unsigned int Prio;//优先级
	int ServiceTime;//进程服务时间
	int RunTime;//已经占用CPU时间
};




enum  schedulestrategies { FCFS, SJF, RR, PRIO } ; // 调度类型:	先来先服务	最短剩余时间优先	时间片轮转	优先级调度


class PM
{
public:
	PM(int t = 0, schedulestrategies s = RR, int tp = 2) // 构造函数 默认当前时间0，RR调度算法，时间片为2。
	{
		currenttime = t;
		strategy = s;
		currentnumproc = 0;
		currentpagenum = 0;
		timepiece = tp;
		currentPID = 0;
		currentproc = readylist.begin();
	}
	list<PCB> readylist; //运行及就绪队列 即占用内存的队列
	list<PCB> waitlist; // 等待队列，在创建过程中由于内存不够等原因而无法进入内存的进程

	unsigned int createPID(); // 维护一个PID池，返回PID
	

	void addproc(string path); // 添加进程
	void killproc(unsigned int PID); // 强制结束进程
	void scheduleproc(); // 调度进程

	vector<PCB_Show> showreadylist();
	vector<PCB_Show> showwaitlist();

private:
	unsigned int currentPID;
	int currenttime; //当前时间
	int currentnumproc; // 当前进程数 包括 readylist 和 waitlist
	int currentpagenum; // 当前所有进程已占用页数 即readylist中进程页数和
	schedulestrategies strategy; // 调度策略
	int timepiece; // 时间片大小

	list<PCB>::iterator currentproc;
	unsigned int currentPID;
};


