#pragma once
#include<stdio.h>
#include<stdlib.h>
#include "file_management.h"
#define BLOCK_SIZE 64//数据块大小，单位：字节
#define MAX_FILE_SIZE 256 //最大文件长度，单位：字节
#define MAX_FILE_NUM 256 //最大文件数量，由此标定inode区存储inode的最大数量
#define MAX_BLOCK 1024 //磁盘块数量（代表了磁盘容量）
#define INODE_START 1 //inode区起始块号（0为第一块）
#define BITMAP_START 129//bitmap区起始块号（0为第一块）
#define DATA_START (BITMAP_START + (sizeof(char)*MAX_BLOCK)/BLOCK_SIZE) //data区起始块号
#define OS_SEEK_SET 0 //标志位，将文件指针定位至文件起始
#define OS_SEEK_END 1 //标志位，将文件指针定位至文件末尾
#define DEV_NAME "disk.bin"

//inode大小为36，dir大小为8


/*
磁盘空间分配如下
 _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
|superblock | inode 节点  | bitmap  | data | ......| data | log | ...| log |
ˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉ

*/


//char bitmap[MAX_BLOCK];//位图用于空闲块的管理



typedef struct super_block {  //超级块结构
	unsigned short s_ninodes; //磁盘中inode节点数
	unsigned short s_nblocks; //磁盘中的物理块数
	unsigned long s_max_size; //文件的最大长度
}super_block;

////磁盘IO部分
//磁盘格式化，向磁盘文件中写入'0'，成功返回True
bool disk_format();
//磁盘初始化，初始化MBR，super_block，inode_table等部分，成功返回True
bool disk_init();
//磁盘启动，读入系统所需的各个参数
bool disk_activate();
//物理块写入函数，buf应为256字节大小的数据块
bool block_write(long block, char *buf);
//物理块读入函数，返回读入的字节大小
bool block_read(long block, char *buf);

////文件读写部分
//获取文件大小
int get_file_size(os_file *fp);
//面向进程的读文件接口
int os_fread(void *v_buf, int size, os_file *fp);
//面向进程的写文件接口
int os_fwrite(void *v_buf, int size, os_file *fp);
//文件指针定位
int os_fseek(os_file *fp, int off_set, int flag);

////目录部分
//面向文件系统的读目录接口
void get_dir(void *dir_buf, iNode *f_inode);

////空闲块管理
//找到第一个空闲块号，修改其bitmap表示被占用，并将其返回
int alloc_first_free();
//释放一个被占用的块
int free_block(int blk);
