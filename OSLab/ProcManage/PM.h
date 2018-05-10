#pragma once

#include<list>
#include<vector>
#include"PCB.h"

using namespace std;


#define MAXMEMORYSIZE 10240//最大物理内存(Bytes)  10MB
#define MAXPAGESIZE 32//最大页大小(Bytes) 
#define MAXSWAPFILESIZE 20480 //最大分页文件大小 20MB

#define MAXPROC 64 //最大进程数 readlist和waitlist的总进程数

#define MAXPROCMEM 160 // 单个进程最大占用内存 160Bytes //目前没用


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
		currentmemory = 0;
		timepiece = tp;
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
	int currenttime; //当前时间
	int currentnumproc; // 当前进程数 包括 readylist 和 waitlist
	int currentmemory; // 当前所有进程已占用内存大小
	schedulestrategies strategy; // 调度策略
	int timepiece; // 时间片大小
};


unsigned int PM::createPID()
{


}


void PM::addproc(string path)
{
	if (currentnumproc >= MAXPROC)
	{
		// 无法创建进程 输出log或 提醒用户
		return;
	}

	os_file* f = Open_File(path);
	if (f) // 文件存在
	{
		int buf[2];
		if (os_fread(buf, sizeof(buf), f)) // 读取成功 读取前8个字节
		{
			int memsize = buf[0];
			int servicetime = buf[1];
			if (memsize == 0 || servicetime == 0)
			{
				//申请内存为0 或者 服务时间为0
				// log 创建进程参数有误
				return;
			}
			vector<int> mem = osMalloc(memsize);
			unsigned int PID = createPID();

			int pos = path.rfind('/');
			string name = path.substr(pos + 1);

			PCB p(PID, path, name, f, mem, currenttime, servicetime);
			if (mem.size == 0)
			{
				p.State = WAITING;
				waitlist.push_back(p);
			}
			else
			{
				p.State = READY;
				readylist.push_back(p);
			}
			

		}
	}
	else
	{
		// 文件不存在
	}
}

void PM::killproc(unsigned int PID)
{

}


/*
进程调度函数
刷新readylist， 将其中状态为FINISH的进程移出，释放相关资源：PID，内存，打开文件等；计算相关资源利用率；并写入log。
刷新readylist和waitlist，将状态为DIED的进程移出，释放PID，打开文件，readylist中的还需释放内存；写入log。
遍历waitlist，为waitlist中可行的进程（内存可申请）申请内存，移出waitlist，移入readylist末尾，将状态改为ready。
根据调度策略从readylist中选取一个合适的进程放在 readylist 头部，表示当前时间片运行该选中程序
刷新readlist
*/
void PM::scheduleproc()
{

}


// 以vector<PCB_Show>返回readylist中的PCB
vector<PCB_Show> PM::showreadylist()
{
	vector<PCB_Show> pr;
	for (list<PCB>::iterator i = readylist.begin(); i != readylist.end(); i++)
	{
		PCB_Show p;
		p.PID = i->PID;
		p.Name = i->Name;
		p.Size = i->Size;
		p.State = i->State;
		p.Prio = i->Prio;
		p.ServiceTime = i->ServiceTime;
		p.RunTime = i->RunTime;
		pr.push_back(p);
	}

	return pr;
}


// 以vector<PCB_Show>返回waitlist中的PCB
vector<PCB_Show> PM::showwaitlist()
{
	vector<PCB_Show> pw;
	for (list<PCB>::iterator i = waitlist.begin(); i != waitlist.end(); i++)
	{
		PCB_Show p;
		p.PID = i->PID;
		p.Name = i->Name;
		p.Size = i->Size;
		p.State = i->State;
		p.Prio = i->Prio;
		p.ServiceTime = i->ServiceTime;
		p.RunTime = i->RunTime;
		pw.push_back(p);
	}

	return pw;
}
