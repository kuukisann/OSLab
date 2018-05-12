#pragma once
#include<vector>
#include<string>
#include "../filesystem/disk.h"

using namespace std;
/*
struct os_file
{

};

os_file* Open_File(string path)
{

}

void Close_File(os_file* f)
{

}

int os_fread(void* buf, int size, os_file* fp)
{

}

vector<int> osMalloc(int memSize)
{

}

bool osFree(vector<int> memBlock)
{

}

void osActive(int memBlockIndex)
{

}
*/

enum procstate { NEW, READY, RUNNING, WAITING, FINISH, DIED}; // 进程状态：新建 就绪 运行 等待 完成 死亡
// 进程默认初始状态 NEW，即waitlist中的进程（假进程，未进入内存）的状态均为NEW；
// 当进程第一次进入内存时，状态由NEW变为READY；
// 当进程第一次运行时，状态由READY变为RUNNING；
// 当进程被调度时，状态由RUNNING变为WAITING 或者 由RUNNING变为WAITING；
// 当进程完成时；状态由RUNNING变为FINISH；


class PCB
{
public:
	PCB(){}
	PCB(unsigned int PID)
	{
		this->PID = PID;
	}
	PCB(
		unsigned int PID, string Path, string Name, os_file* f, int needmemsize,
		vector<int> Mem, unsigned int Prio, int ArriveTime, int ServiceTime);
	~PCB(){}

	unsigned int PID; //进程PID
	string Path; //进程所在文件路径
	string Name; // 进程名称
	os_file* procftr; // 进程文件指针

	int needmemsize; // 进程需要内存大小(Bytes)
	vector<int> Mem; //进程内存页号
	int Size; //进程占用内存页数
	procstate State;  //进程状态
	unsigned int Prio;//优先级

	int ArrivalTime; //进程创建时间  
	int StartTime;  //进程开始运行时间  
	int ServiceTime;//进程服务时间
	int RunTime;//已经占用CPU时间
	int NeedTime;//还要占用CPU时间
	int FinishTime;//进程结束时间

	bool operator == (const PCB &a)const
	{
		return PID == a.PID ? true : false;
	}

	bool operator <(const PCB &a)const
	{
		return PID < a.PID;
	}

private:
};

inline PCB::PCB(
	unsigned int PID, string Path, string Name, os_file* f, int needmemsize, 
	vector<int> Mem, unsigned int Prio, int ArrivalTime, int ServiceTime
	)
{
	this->PID = PID;
	this->Path = Path;
	this->Name = Name;
	this->procftr = f;

	this->needmemsize = needmemsize;
	this->Mem = Mem;
	this->Size = Mem.size();
	this->State = NEW;
	this->Prio = Prio;
	
	this->ArrivalTime = ArrivalTime;
	this->StartTime = -1; //-1 表示进程从未开始运行
	this->ServiceTime = ServiceTime;
	this->RunTime = 0;
	this->NeedTime = ServiceTime;
	this->FinishTime = -1; //-1 表示进程未完成
}

inline bool sortbyprio(const PCB & p1, const PCB & p2)
{
	return p1.Prio < p2.Prio;
}

inline bool sortbyneedtime(const PCB & p1, const PCB & p2)
{
	return p1.NeedTime < p2.NeedTime;
}


