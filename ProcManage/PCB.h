#pragma once
#include<vector>
using namespace std;

/*接口函数
struct os_file
{

};

os_file* Open_File(string path)
{

}

int os_fread(void* buf, int size, os_file* fp)
{

}

vector<int> osMalloc(int memSize)
{

}
*/

enum procstate { NEW, READY, RUNNING, WAITING, FINISH, DIED}; // 进程状态：新建 就绪 运行 等待 完成 死亡

class PCB
{
public:
	PCB();
	PCB(
		unsigned int PID, string Path, string Name, os_file* f,
		vector<int> Mem, int ArriveTime, int ServiceTime);
	~PCB();

	unsigned int PID; //进程PID
	string Path; //进程所在文件路径
	string Name; // 进程名称
	os_file* procftr; // 进程文件指针

	vector<int> Mem; //进程内存块
	int Size; //进程占用内存块大小
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



PCB::PCB(){} // 空构造函数

PCB::PCB(
	unsigned int PID, string Path, string Name, os_file* f,
	vector<int> Mem, int ArrivalTime, int ServiceTime
	)
{
	this->PID = PID;
	this->Path = Path;
	this->Name = Name;
	this->procftr = f;

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

PCB::~PCB(){}


