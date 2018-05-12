#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<malloc.h>
#include "disk.h"
#include "file_management.h"


//磁盘格式化，实际操作为重新建立模拟磁盘文件
bool disk_format()
{
	FILE *pdisk;
	char *buf = (char*)malloc(MAX_BLOCK * BLOCK_SIZE);
	if (pdisk = fopen(DEV_NAME, "wb")) {
		fwrite(buf, MAX_BLOCK * BLOCK_SIZE, 1, pdisk); //将申请的足够大的内存写入文件
		fclose(pdisk);
		return true;
	}
	else {
		//printf("Error, Cannot format the disk"); //Log输出！！！！
		return false;
	}
}

//磁盘初始化
bool disk_init()
{
	//超级块初始化
	super_block *sb = (super_block*)malloc(sizeof(super_block));
	sb->s_ninodes = iNode_NUM; 
	sb->s_nblocks = MAX_BLOCK;
	sb->s_max_size = MAX_FILE_NUM;

	//inode初始化
	iNode *i_table = (iNode*)malloc(iNode_NUM * sizeof(iNode));
	for (int i = 0; i < iNode_NUM; i++) {
		init_iNode(i_table + i);
	}
	
	//bitmap初始化
	char *bit_map = (char*)malloc(MAX_BLOCK);
	int cannot_use = DATA_START - 1; //无法使用的块编号（从0开始）
	for (int i = 0; i < MAX_BLOCK; i++) { //对bitmap进行初始化
		if (i <= cannot_use)
			*(bit_map + i) = 1;
		else
			*(bit_map + i) = 0;
	}
	//root_dir初始化
	dir root[DIR_NUM];
	init_dir(root);
	iNode root_iNode = Fill_in_iNode(0);
	for (int i = 0; i < FBLK_NUM; i++) {
		root_iNode.block_address[i] = cannot_use + 1 + i;
		*(bit_map + cannot_use + i + 1) = 1;
	}
	*i_table = root_iNode;
	//写入磁盘
	FILE *disk = fopen(DEV_NAME, "rb+"); //以读写方式打开文件，不会清空文件
	fwrite(sb, sizeof(super_block), 1, disk); //写入超级块内容
	fseek(disk, INODE_START * BLOCK_SIZE, SEEK_SET);
	fwrite(i_table, sizeof(iNode) * iNode_NUM, 1, disk); //写入inode_table
	fseek(disk, BITMAP_START * BLOCK_SIZE, SEEK_SET);
	fwrite(bit_map, MAX_BLOCK, 1, disk); //写入bitmap
	for (int i = 0; i < FBLK_NUM; i++) {
		fseek(disk, root_iNode.block_address[i] * BLOCK_SIZE, SEEK_SET);
		fwrite((char*)root + i * BLOCK_SIZE, BLOCK_SIZE, 1, disk);
	} //root_dir写入磁盘
	//测试
	char *bp = (char*)malloc(MAX_BLOCK);
	fseek(disk, BITMAP_START * BLOCK_SIZE, SEEK_SET);
	fread(bp, MAX_BLOCK, 1, disk);
	//for (int i = 0; i < MAX_BLOCK; i++)
	//	cout << (int)(*(bp + i)) << " ";
	free(bp);
	free(sb);
	free(i_table);
	free(bit_map);
	fclose(disk);
	return true;
}

bool disk_activate()
{
	dir *tmp_root = root_dir;
	FILE *fp = fopen(DEV_NAME, "rb");
	if (fp) {
		fseek(fp, INODE_START * BLOCK_SIZE, SEEK_SET);
		fread(&iNode_table, iNode_NUM * sizeof(iNode), 1, fp);//读入iNode_table
		for (int i = 0; i < FBLK_NUM; i++) {
			fseek(fp, iNode_table[0].block_address[i] * BLOCK_SIZE, SEEK_SET);
			fread((char*)tmp_root + i * BLOCK_SIZE, BLOCK_SIZE, 1, fp);
		} //读入root_dir
		current_dir = root_dir;
		fclose(fp);
		return true;
	}
	else
		return false;
}



//整块逻辑块写入物理块
bool block_write(long block, char *buf)
{
	FILE * disk = fopen(DEV_NAME, "ab+");
	if (disk) {
		fseek(disk, (block * BLOCK_SIZE), SEEK_SET); // 定位到相应的块的位置
		fwrite(buf, BLOCK_SIZE, 0, disk);
		fclose(disk);
		return true;
	}
	else {
		//printf("Error: Cannot operate on disk!\n"); //改成log！！！
		return false;
	}
}

//整块物理块读入缓冲区
bool block_read(long block, char *buf)
{
	FILE * disk = fopen(DEV_NAME, "r");
	if (disk) {
		fread(buf, BLOCK_SIZE, 0, disk);
		fclose(disk);
		return true;
	}
	else {
		//printf("Error: Cannot operate on disk!\n"); //改成log
		return false;
	}
}



//获取文件大小
int get_file_size(os_file *fp)
{
	return fp->f_iNode->i_size;
}
//文件读函数
int os_fread(void *v_buf, int size, os_file *fp)
{
	int blk_start, blk_num, off_set;
	char *buf = (char*)v_buf;
	//memcpy(buf, v_buf, size); //新建一个buf变量用于对内存区操作
	blk_start = fp->f_pos / BLOCK_SIZE; //从文件的第几个block开始读,从0开始计
	blk_num = (fp->f_pos + size) / BLOCK_SIZE - blk_start + 1; // 读几个block
	off_set = (fp->f_pos + size) % BLOCK_SIZE;//最后一个block的off_set
	if (off_set == 0 && blk_num > 1)
		off_set = BLOCK_SIZE; //偏移为0说明要读入一整块
	//判断要读的数据是否超出文件界限
	if (fp->f_pos + size > fp->f_iNode->i_size) {
		return 0; 
	}

	FILE *disk_in;
	disk_in = fopen(DEV_NAME, "rb");
	if (blk_num == 1) {
		fseek(disk_in, fp->f_iNode->block_address[0] * BLOCK_SIZE + fp->f_pos, SEEK_SET);
		fread(buf, size, 1, disk_in);
	}
	else {
		int read_bytes = 0; //累计已读入字节数
		int cur_bytes = 0; //当前块内需要读取的字节数
		for (int blk = blk_start; blk < blk_num + blk_start; blk++) {
			if (blk == blk_start) { //是第一块
				cur_bytes = BLOCK_SIZE - fp->f_pos % BLOCK_SIZE;
			}
			else if (blk == blk_start + blk_num - 1) { //是最后一块
				cur_bytes = off_set;
			}
			else { //需要读出全部数据的中间块
				cur_bytes = BLOCK_SIZE;
			}
			rewind(disk_in);
			fseek(disk_in, fp->f_pos + fp->f_iNode->block_address[blk] * BLOCK_SIZE + read_bytes, SEEK_SET);
			fread(buf + read_bytes, cur_bytes, 1, disk_in);
			read_bytes += cur_bytes;
		}
	}
	fclose(disk_in);
	//v_buf = (void*)buf;
	return 1;
}
//文件写函数
int os_fwrite(void *v_buf, int size, os_file *fp)
{
	//计算要写的块数及偏移量
	int blk_start, blk_num, off_set, total_blk;
	char *buf = (char*)malloc(size);
	memcpy(buf, v_buf, size); //新建一个buf变量用于对内存区操作
	total_blk = (fp->f_pos + size) / BLOCK_SIZE + 1; //写入文件后文件的block数
	blk_start = fp->f_pos / BLOCK_SIZE; //从文件的第几个block开始写,从0开始计
	blk_num = (fp->f_pos + size) / BLOCK_SIZE - blk_start + 1; // 写几个block
	off_set = (fp->f_pos + size) % BLOCK_SIZE;//最后一个block的off_set
	if (off_set == 0 && blk_num > 1)
		off_set = BLOCK_SIZE; //偏移为0且写数据块大于一说明要写入一整块

	if (total_blk > FBLK_NUM) { //写入数据块数超出文件的最大块数
		return 0;
	}
	//如果需要，将空闲块分配给文件
	for (int i = 0; i < total_blk; i++) {
		if (fp->f_iNode->block_address[i] > MAX_BLOCK) { //说明文件的第i块尚未被分配
			fp->f_iNode->block_address[i] = alloc_first_free();
			if (fp->f_iNode->block_address[i] == -1) {
				fp->f_iNode->block_address[i] = MAX_BLOCK + 1;
				return 0; //没有空闲块可以分配
			}
		}
	}
	//直接写磁盘
	FILE *disk_p;
	disk_p = fopen(DEV_NAME, "rb+"); //打开磁盘文件
	if (blk_num == 1) {
		rewind(disk_p);
		fseek(disk_p, fp->f_pos+fp->f_iNode->block_address[blk_start]*BLOCK_SIZE, SEEK_SET);
		//定位文件指针
		fwrite(buf, size, 1, disk_p);
		//测试用
		char *tmp_buf = (char*)malloc(size);
		fseek(disk_p, fp->f_pos + fp->f_iNode->block_address[blk_start] * BLOCK_SIZE, SEEK_SET);
		fread(tmp_buf, size, 1, disk_p);
		//cout << *(tmp_buf) << endl;
		free(tmp_buf);
	}
	else {
		int write_bytes = 0; //已写入的字节数
		int cur_bytes = 0; //当前块内需要写入的字节数
		for (int blk = blk_start; blk < (blk_start + blk_num); blk++) {
			if (blk == blk_start) { //是第一块
				cur_bytes = BLOCK_SIZE - fp->f_pos % BLOCK_SIZE;
			}
			else if (blk == blk_start + blk_num - 1) { //是最后一块
				cur_bytes = off_set;
			}
			else { //需要读出全部数据的中间块
				cur_bytes = BLOCK_SIZE;
			}
			rewind(disk_p);
			fseek(disk_p, fp->f_pos + fp->f_iNode->block_address[blk] * BLOCK_SIZE + write_bytes, SEEK_SET);
			fwrite(buf + write_bytes, cur_bytes, 1, disk_p);
			write_bytes += cur_bytes;
		}
	}
	fp->f_pos += size; //更新文件指针
	fp->f_iNode->i_size += size; //更新文件大小
	fclose(disk_p); //关闭磁盘读写
	free(buf);
	return 1;
}
//文件指针定位
int os_fseek(os_file *fp, int off_set, int flag)
{
	if (flag == 0) { //标志位为0，从文件起始位置移动off_set个字节
		if (off_set <= fp->f_iNode->i_size) {
			fp->f_pos = off_set;
			return 1;
		}
		else
			return 0;
	}
	else if (flag == 1) { //标志位为1，指针移动至文件末尾
		if (off_set == 0) {
			fp->f_pos = fp->f_iNode->i_size;
			return 1;
		}
		else
			return 0;
	}
}

//面向文件系统的读目录接口
void get_dir(void *dir_buf, iNode *f_inode)
{
	//根据计算可知一个目录就占32字节,一个目录下8个文件相当于4个block
	//int blk_nr, off_set;
	//blk_nr = DIR_NUM * sizeof(dir) / BLOCK_SIZE + 1;
	//off_set = DIR_NUM * sizeof(dir) % BLOCK_SIZE;
	int read_bytes = 0; //已读字节数
	FILE *disk_dir = fopen(DEV_NAME, "rb");
	if (disk_dir) {
		char *buf = (char*)dir_buf;
		for (int i = 0; i < FBLK_NUM; i++) {
			fseek(disk_dir, f_inode->block_address[i] * BLOCK_SIZE, SEEK_SET); //定位文件指针
			fread(buf + i * BLOCK_SIZE, BLOCK_SIZE, 1, disk_dir);
		}
		//fclose(disk_dir);
	}
}


//找到第一个空闲块号，修改其bitmap表示被占用，并将其返回
int alloc_first_free()
{
	char *bitmap = (char*)malloc(MAX_BLOCK);
	FILE *fp = fopen(DEV_NAME, "rb+");
	fseek(fp, BITMAP_START * BLOCK_SIZE, SEEK_SET);
	fread(bitmap, MAX_BLOCK, 1, fp);
	for (int i = 0; i < MAX_BLOCK; i++) {
		//cout << (int)(*(bitmap + i)) << " ";
		if (*(bitmap + i) == 0) {
			*(bitmap + i) = 1; //修改bitmap表示该块被占用
			fseek(fp, BLOCK_SIZE * BITMAP_START, SEEK_SET); //文件指针回到bitmap起点
			fwrite(bitmap, MAX_BLOCK, 1, fp); //将bitmap写回磁盘
			fclose(fp);
			free(bitmap);
			return i;
		}
	}
	fclose(fp);
	free(bitmap);
	return -1;
}
//释放一个被占用的块
int free_block(int blk)
{
	char *bitmap = (char*)malloc(MAX_BLOCK);
	char *blk_format = (char*)malloc(BLOCK_SIZE); //用于磁盘块的格式化

	FILE *fp = fopen(DEV_NAME, "rb+");
	fseek(fp, BLOCK_SIZE * BITMAP_START, SEEK_SET);
	fread(bitmap, MAX_BLOCK, 1, fp); //从磁盘上读入bitmap
	if (blk <= DATA_START) {
		return 0; //如果释放不可用区的块，报错，可输出至log
	}
	*(bitmap + blk) = 0; //bitmap置0
	fseek(fp, BLOCK_SIZE * BITMAP_START, SEEK_SET); //指针准备写入bitmap
	fwrite(bitmap, MAX_BLOCK, 1, fp); //将bitmap写回磁盘
	fseek(fp, BLOCK_SIZE * blk, SEEK_SET); //将指针移动到要格式化的磁盘块位置
	fwrite(blk_format, BLOCK_SIZE, 1, fp); //磁盘块格式化，相当于释放
	free(bitmap);
	free(blk_format);
	fclose(fp);
	return 1;
}

