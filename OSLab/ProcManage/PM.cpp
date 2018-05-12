#include<ctime>
#include<iostream>
#include<cstdlib>
#include<ctime>
#include"PM.h"



unsigned int PM::createPID()
{
	return ++currentPID;
}


void PM::addproc(string path)
{
	mu.lock();
	if (currentnumproc >= MAXPROC)
	{
		//log无法创建进程，进程数已满
		mu.unlock();
		return;
	}

	if (currentpagenum >= MAXPAGENUM)
	{
		// log无法创建进程，内存已满
		mu.unlock();
		return;
	}

	os_file* f = Open_File(path);
	if (f) // 文件存在
	{
		int buf[3];
		if (os_fread(buf, sizeof(buf), f)) // 读取成功 读取前8个字节
		{
			int needmemsize = buf[0];
			int servicetime = buf[1];
			int datablocksize = buf[2];
			if (needmemsize == 0 || servicetime == 0)
			{
				//申请内存为0 或者 服务时间为0
				// log 创建进程参数有误
				Close_File(f);
				mu.unlock();
				return;
			}

			if (needmemsize < datablocksize)
			{

				//log("内存大小小于数据大小，进程文件参数错误")
				Close_File(f);
				mu.unlock();
				return;
			}
			vector<int> mem = PMP->osMalloc(needmemsize);
			unsigned int PID = createPID();

			int pos = path.rfind('/');
			string name = path.substr(pos + 1);

			PCB p(PID, path, name, f, needmemsize, mem, (unsigned int)rand() % 100, currenttime, servicetime);
			currentnumproc++; // 进程数+1
			if (mem.size() == 0)
			{
				waitlist.push_back(p);
			}
			else
			{
				p.State = READY;
				readylist.push_back(p);
				currentpagenum += p.Size; // 更新内存页数

				os_fseek(f, 12, OS_SEEK_SET);//第一次初始化指针到数据块开始的位置

				if (datablocksize < PAGESIZE)
				{
					char databuf[PAGESIZE] = { 0 };
					os_fread(databuf, datablocksize, f);
					PMP->setBlockContent(p.Mem[0], databuf);
				}
				else
				{
					for (int i = 0; i < int(datablocksize / PAGESIZE) + 1; i++)
					{
						char databuf[PAGESIZE] = { 0 };
						if (i == int(datablocksize / PAGESIZE)) // 最后一块
						{

							os_fread(databuf, datablocksize%PAGESIZE, f);
							PMP->setBlockContent(p.Mem[i], databuf);
						}
						else
						{
							os_fread(databuf, PAGESIZE, f);
							PMP->setBlockContent(p.Mem[i], databuf);
						}
					}
				}
			}
		}
		Close_File(f);
	}
	else
	{
		// log("文件不存在")
		mu.unlock();
		return;
	}
	mu.unlock();
}

void PM::killproc(const unsigned int PID)
{
	mu.lock();
	PCB p;
	p.PID = PID;
	list<PCB>::iterator i = find(readylist.begin(), readylist.end(), p);
	if (i == readylist.end()) //在readylist里未找到
	{
		i = find(waitlist.begin(), waitlist.end(), PID);
		if (i == waitlist.end())
		{
			//log不存在该进程
			mu.unlock();
			return;
		}
		else
		{
			i->State = DIED;
			//log
		}
	}
	else
	{
		if (i == currentproc)
		{
			currentproc = readylist.begin();
		}
		i->State = DIED;
		//log
	}
	mu.unlock();

}


/*
进程调度函数
//刷新readylist， 将其中状态为FINISH的进程移出，释放相关资源：PID，内存，打开文件等；计算相关资源利用率；并写入log。

//刷新readylist和waitlist，将状态为DIED的进程移出，释放PID，打开文件，readylist中的还需释放内存；写入log。

遍历waitlist，为waitlist中可行的进程（内存可申请）申请内存，移出waitlist，移入readylist末尾，将状态改为ready。

根据调度策略从readylist中选取一个合适的进程放在 readylist 头部，表示当前时间片运行该选中程序。
for(int i = 0;i<currentproc->Size; i++)
{
osActive(currentproc->Mem[i]);
}

//可以sleep


刷新readlist
*/
void PM::scheduleproc()
{
	mu.lock();
	// RR调度算法

	// 移出readylist中的FINISH和DIED进程
	if (!readylist.empty())
	{
		for (list<PCB>::iterator i = readylist.begin(); i != readylist.end();) // 不在此处i++的原因是，erase之后会导致迭代器i指向后一个
		{
			if (i->State == FINISH || i->State == DIED) // 进程在上个时间片已完成
			{
				PMP->osFree(i->Mem); // 释放进程内存
				currentpagenum -= i->Size; // 更新内存页数
				i = readylist.erase(i); // 移出队列 
				currentnumproc--; // 进程数减1 
								  //log 进程PCB 已退出内存

			}
			else // next
			{
				i++;
			}
		}
	}


	
	if (!waitlist.empty())
	{
		// 移出waitlist中的DIED进程
		for (list<PCB>::iterator i = waitlist.begin(); i != waitlist.end();)
		{
			if (i->State == DIED) // 进程被Kill
			{
				PMP->osFree(i->Mem); // 释放进程内存
				currentpagenum -= i->Size; // 更新内存页数
				i = waitlist.erase(i); // 移出队列 
				currentnumproc--; // 进程数减1 
								  //log 进程PCB 已退出

			}
			else // next
			{
				i++;
			}
		}

		// 将waitlist中内存可行的进程调入内存
		for (list<PCB>::iterator i = waitlist.begin(); i != waitlist.end();)
		{
			if ((i->Mem = PMP->osMalloc(i->Needmemsize)).size()) // 申请到了内存
			{
				PCB temp = *i;

				temp.Size = temp.Mem.size();
				temp.State = READY;

				readylist.push_back(temp);
				currentpagenum += temp.Size; // 更新内存页数
				waitlist.erase(i);
			}
			else
			{
				i++;
			}
		}
	}


	// 调度
	if (!readylist.empty() && readylist.begin()->ArrivalTime <= currenttime) //就绪队列不为空且满足时间条件
	{

		// 运行队列第一个进程
		currentproc = readylist.begin();

		// 进程第一次进入内存，还未运行过，设置开始运行时间StartTime
		if (currentproc->State == READY || currentproc->StartTime == -1)
		{
			currentproc->StartTime = currenttime;
		}

		currentproc->State = RUNNING;
		for (int i = 0; i < currentproc->Size; i++) // 激活要运行进程的内存页
		{
			PMP->osActive(currentproc->Mem[i]);
		}

		// 当前时间片是否可以运行完成
		if (currentproc->NeedTime <= timepiece) // 进程需要时间小于一个时间片
		{
			currenttime += currentproc->NeedTime; // 更新当前时间
			currentproc->RunTime += currentproc->NeedTime;
			currentproc->FinishTime = currenttime; // 记录进程完成时间
			currentproc->NeedTime = 0;
			currentproc->State = FINISH;
			// log 进程current->PID已完成
		}
		else
		{
			currenttime += timepiece; // 更新当前时间
			currentproc->RunTime += timepiece;
			currentproc->NeedTime -= timepiece;
			currentproc->State = WAITING;
		}


		// 移到队尾
		PCB p = *currentproc;
		readylist.push_back(p);
		readylist.erase(currentproc);
		currentproc = readylist.begin();
	}
	else
	{
		//log 当前时间：currentime 无进程运行
		currenttime += timepiece;
	}
	mu.unlock();
}


// 以vector<PCB_Show>返回readylist中的PCB
vector<PCB_Show> PM::showreadylist()
{
	mu.lock();
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
	mu.unlock();
	return pr;
}


// 以vector<PCB_Show>返回waitlist中的PCB
vector<PCB_Show> PM::showwaitlist()
{
	mu.lock();
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
	mu.unlock();
	return pw;
}

void PM::run()
{
	clock_t strat = clock();
	while (!isExit)
	{
		if (clock() - strat >= timepiece * 1000)
		{
			strat = clock();
			scheduleproc();
			//cout << "schedule " << clock() << endl;
		}
	}
	cout << "PM thread endded\n";
}
