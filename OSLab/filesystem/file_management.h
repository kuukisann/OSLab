#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

#define Name_length 14 //文件名称最大长度  
#define iNode_NUM 256 //iNode的数量
#define DIR_NUM 8 //每个目录文件下的文件最大个数
#define PATH_LENGTH 100 //路径字符串最大长度 
#define MAX_BLOCK 1024 //磁盘块数量
#define FBLK_NUM 4 //文件中block的个数 
#define CURRENT_TIME 123  //当前时间，待定 
//打开文件的模式f_mode 
#define RDONLY 00 //只读
#define WRONLY 01 //只写
#define RDWR 02 //读写 



//iNode
typedef struct INODE{
	unsigned short i_mode;// 文件类型，0目录，1普通 
	int i_size;//文件大小（字节数） 
	//int permission;//文件的读、写、执行权限
	/*int ctime; //文件的时间戳，inode上一次变动的时间
	int mtime; //文件的时间戳，文件内容上一次变动的时间
	int atime;//文件的时间戳，文件上一次打开的时间。*/
	int nlinks; //链接数，即有多少文件目录项指向这个inode 判断一个 i节点是否应该被释放
	int block_address[FBLK_NUM];//文件数据block的位置             
	int open_num; //0没有被打开，1已经被打开，防止多次打开文件 
}iNode; 
 
//文件目录项结构:当前目录下一系列目录项的列表 
typedef struct directory{
	string file_name;//文件名
	unsigned int iNode_no; //iNode编号 
}dir;

//文件的句柄 
typedef struct OS_FILE{
	//unsigned short f_mode;//文件操作模式（RW位） ------------待定的------------- 
	//unsigned short f_flag;
	//unsigned short f_count;//对应文件引用计数值 
	iNode *f_iNode;//指向对应iNode 
	long int f_pos;//读指针 
}os_file;

////全局变量
extern iNode iNode_table[iNode_NUM];//iNode table的数组，数组下标对应iNode编号 
extern dir root_dir[DIR_NUM];//根目录 数组实现  往下的每个子目录也是dir类型的数组，每一项是一个文件目录项 
extern dir* current_dir;//保存每次更新analyse_path返回的dir数组，即当前目录的dir数组 

void init_iNode(iNode* blankiNode); //iNode初始化函数
void format_iNode(iNode* blankiNode); //iNode格式化函数 
void init_dir(dir blankdir[]); //dir数组初始化函数 

//string path;//记录路径 

iNode* Create_File(string pathname, unsigned short f_type); //创建文件 返回类型先暂定 
int Delete_File(string f_name, int f_i, string path); //删除文件 rm filename (返回值：成功0，失败-1) 
os_file* Open_File(string f_name); //打开文件 
void Close_File(os_file	*f); //关闭文件 

unsigned int dirlookup(string son_name, dir father[]);//根据子名称，找到子iNode编号 
unsigned int get_empty_iNode(); //找到空闲的iNode,返回编号（数组下标） 
iNode Fill_in_iNode(unsigned short f_type);//完善iNode信息 
dir* analyse_Path(string path);//解析路径，从左到右找到当前目录/文件名称 
int find_dir_no(string f_name); //从current_dir中查找该文件的dir在目录的数组下标 
void get_dir(void *dir_buf, iNode *f_iNode);

//创建目录文件 mkdir dirName  调用Create_File 
//删除普通文件 rm filename 调用Delete_File 
//删除目录和目录下所有文件 rmdir dirName  调用os_rmdir()



//////////////这几个都是在当前目录下的操作，所以进程那边得沟通好/////////////// 
int os_rm(string f_name, string path); //删除文件
bool os_rmdir(string dir_name, string path); //删除目录和目录下所有文件
vector<string> os_ls(); //列出目录下所有文件
bool os_cd(string &currentpath, string newpath);// 切换目录  上下切换 or 给出全路径自动切换


//从上一次后读取size字节的文件内容
//从头开始读取size字节的文件内容
//从文件尾写入内容
//清空文件，从头写入
