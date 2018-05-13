#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_management.h"
#include <iostream>
#include <string>
#include "disk.h"
using namespace std;

//iNode初始化函数 
void init_iNode(iNode* blankiNode) {
	blankiNode->i_mode = 2;
	blankiNode->i_size = 0;
	//blankiNode->permission = -1;
	blankiNode->nlinks = 0;
	blankiNode->open_num = 0;
	int i;
	for (i = 0; i<FBLK_NUM; i++) {
		if (blankiNode->block_address[i] < MAX_BLOCK) {
			blankiNode->block_address[i] = MAX_BLOCK + 1;
		}
		else
			break;
	}
}

//iNode格式化函数 
void format_iNode(iNode* blankiNode){
	blankiNode->i_mode = 2;
	blankiNode->i_size = 0;
	//blankiNode->permission = -1;
	blankiNode->nlinks = 0;
	blankiNode->open_num = 0;
	int i;
	for(i=0;i<FBLK_NUM;i++){
		if (blankiNode->block_address[i] < MAX_BLOCK) {
			free_block(blankiNode->block_address[i]); //如果块被占用则释放块,修改bitmap
			blankiNode->block_address[i] = MAX_BLOCK + 1;
		}
		else
			break;
	}
}

//判断是否有重名文件
int same_name(string f_name,dir item[]){
	int i;
	for(i=0;i<DIR_NUM && item[i].file_name!=f_name && item[i].iNode_no!=iNode_NUM+1;i++){}
	if(item[i].iNode_no==iNode_NUM+1 || i==DIR_NUM) 
		return 0; //遍历一遍，没有重名
	else
		return -1; //有重名 	
}

//解析绝对路径，从左到右找到当前目录/文件的dir* , cur_iNode用于接收返回的iNode
dir* analyse_Path(string path, int *i_no){  
	int pos, i;
	string father, son;
	string temp = path.substr(1);
	pos = temp.find('/');
	father = temp.substr(0,pos); //路径从头到第一个斜杠的子串，即最左目录名，此时是root
	temp = temp.substr(pos+1); //路径去掉最左目录名 
	dir* son_dir = (dir*)malloc(sizeof(dir)*DIR_NUM);
	dir* father_dir = root_dir; //第一次是root_dir，之后是父目录的dir数组
	iNode find_iNode;
	int node;
	if (temp == "root") {
		current_dir = father_dir;
		*i_no = 0;
	}
	else {
		while(temp.find('/') != string::npos){ //查找成功，查找失败则跳出循环				 		
			pos = temp.find('/');
			son = temp.substr(0,pos);
			if (dirlookup(son, father_dir) == iNode_NUM + 1) {
				Log::w("Wrong path.\n");
				return NULL;
			}
			find_iNode = iNode_table[dirlookup(son,father_dir)]; //从father中找到son文件的iNode		
			get_dir(son_dir, &find_iNode);//传入iNode结点，返回son文件的dir文件目录项数组son_dir 
			father_dir = son_dir;		 
			father = son;
			temp = temp.substr(pos+1); //路径去掉最左目录名 
		}
		node = dirlookup(temp, father_dir);
		if (node != iNode_NUM + 1) {
			find_iNode = iNode_table[node];//从father中找到temp文件的iNode
		}
		else{
			Log::w("Cannot analyse the path.\n");
			return NULL;
		}
			
		get_dir(son_dir, &find_iNode);//传入iNode结点，返回son文件的文件目录项数组son_dir
		*i_no = node;
		current_dir = son_dir; //更新到表中	
	}
	return current_dir; //最后跳出循环的temp是最后一级的文件的dir数组 
}

//根据子名称，找到子iNode的编号。在这个目录下，没有重名文件，所以可以遍历父dir 
unsigned int dirlookup(string son_name, dir *father){
	int i;
	for(i=0; i<DIR_NUM && son_name!=(*(father+i)).file_name; i++){}
	if(i == DIR_NUM){
		//printf("没有该文件，error！\n");
		return iNode_NUM+1;
	}
	return (*(father+i)).iNode_no; 
} 

//找到空闲的iNode,返回编号（数组下标）
unsigned int get_empty_iNode(){
	int i;
	for(i=0;i<iNode_NUM;i++)
		if(iNode_table[i].nlinks == 0)
			return i;	
	if(i == iNode_NUM)
		return iNode_NUM + 1;
} 
 
//对于创建的文件，完善iNode信息 
iNode Fill_in_iNode(unsigned short f_type){ 
	iNode new_iNode;
	new_iNode.i_mode = f_type; //0目录，1普通	
	if (f_type)
		new_iNode.i_size = 0;
	else
		new_iNode.i_size = 1;
	new_iNode.nlinks = 1; 
	new_iNode.open_num = 0;
	for (int i = 0; i < FBLK_NUM; i++) {
		new_iNode.block_address[i] = MAX_BLOCK + 1;
	}
	return new_iNode;
}

//创建文件 
iNode* Create_File(string pathname, string cur_path, unsigned short f_type){ //参数在Fill_in_iNode上还有一些，讨论之后再加 
	string path, f_name;
	dir* item;
	//iNode *cur_iNode = NULL;
	int cur_i_no = -1;
	if (pathname[0] != '/') {//相对路径，依据current_dir
		f_name = pathname;
		item = current_dir;
		analyse_Path(cur_path, &cur_i_no); //主要为了获得当前目录的相应目录文件的iNode
	}
	else {//绝对路径
		int pos = pathname.rfind('/');
		path = pathname.substr(0, pos);
		f_name = pathname.substr(pos + 1);
		item = analyse_Path(path, &cur_i_no); //解析路径，得到其所在目录文件的dir数组 
	}

	//1.检测文件名长度
	if (f_name.length()> Name_length){
		Log::w("a shorter name is needed\n");
		return NULL;
	}
	//2.判断是否存在同名的文件
	if (item == NULL) {
		Log::w("create_file: Something wrong when analyse the path")
		return NULL;
	}
	if (same_name(f_name, item) == -1) {
		Log::w("The file already exits\n");
		return NULL;
	}
	//3.创建文件目录项，放在文件所在的目录的directory列表中 
	int i;
	for(i=0;i<DIR_NUM && (*(item+i)).iNode_no!=iNode_NUM + 1;i++){} //找到item的尾项///////指针数组的再看看形式//////// 
	unsigned int temp_no = get_empty_iNode();
	if(temp_no == iNode_NUM+1){
		Log::w("There is no iNode that can be used\n"); 
		return NULL;
	}
	else{ //在数组尾项添加文件目录项  	
		(*(item+i)).file_name = f_name;
		(*(item+i)).iNode_no = temp_no;
	}
	
	//4.完善iNode节点信息,并将iNode节点放在iNode table中  
	iNode newiNode = Fill_in_iNode(f_type);
	if (f_type == 0) { //f_type为0时说明创建目录文件
		dir temp_dir[DIR_NUM]; 
		init_dir(temp_dir); //初始化dir
		FILE *disk_p = fopen(DEV_NAME, "rb+");
		int blk_addr;
		//char *temp = (char*)malloc(BLOCK_SIZE); 
		//当前情况下一个目录占四个block
		dir *tmp_buf = temp_dir;
		for (int k = 0; k < FBLK_NUM; k++) {
			blk_addr = alloc_first_free();
			newiNode.block_address[k] = blk_addr;
			fseek(disk_p, blk_addr * BLOCK_SIZE, SEEK_SET);
			fwrite(((char*)tmp_buf + k * BLOCK_SIZE), BLOCK_SIZE, 1, disk_p);
		}
		fclose(disk_p);
	}
	iNode_table[(*(item+i)).iNode_no] = newiNode; 
	
	//(4)写入磁盘，iNode_table更新，item所指的目录进行更新(利用cur_iNode)
	FILE *disk_p = fopen(DEV_NAME, "rb+");
	fseek(disk_p, INODE_START * BLOCK_SIZE, SEEK_SET); //文件指针定位至inode_table处
	fwrite(iNode_table, sizeof(iNode_table), 1, disk_p); //iNode_table更新
	if (cur_i_no != -1) {
		char *buf = (char*)item; //文件目录写回磁盘
		for (int k = 0; k < FBLK_NUM; k++) {
			fseek(disk_p, iNode_table[cur_i_no].block_address[k] * BLOCK_SIZE, SEEK_SET);
			fwrite((buf + k * BLOCK_SIZE), BLOCK_SIZE, 1, disk_p);
		}
	}
	fclose(disk_p);
	if (pathname[0] != '/') {
		current_dir = item;
	} //如果是相对路径不能忘记更新current_dir
	return &newiNode;
}

//打开文件：根据文件名（绝对路径or相对路径），定位到其iNode，修改open_num，创建文件句柄os_file 
os_file* Open_File(string f_name){
	os_file* current_file = (os_file*)malloc(sizeof(os_file));
	//iNode *cur_iNode = NULL;
	int *cur_i_no = NULL;
	if(f_name[0] == '/'){//绝对路径 
		int rpos = f_name.rfind('/'); //找到f_name最后一次出现'/'的位置		 
		dir* current= analyse_Path(f_name.substr(0,rpos), cur_i_no); //得到该文件所在目录的dir*数组 
		if (current == NULL) {
			Log::w("open_file: Cannot find the dir.\n");
			return NULL;
		}
		current_file->f_iNode = iNode_table + dirlookup(f_name.substr(rpos),current);
	}
	else{//相对路径，已经就是这个文件的名字了，此时依据dir* current_dir：文件所在的目录 
		current_file->f_iNode = iNode_table + dirlookup(f_name,current_dir);  
	}
	if(current_file->f_iNode->open_num == 1){ //判断是否已经打开 
		Log::w("The file has been opend\n");
		return NULL;
	}
	current_file->f_pos = 0; //读指针置0 
	current_file->f_iNode->open_num = 1; //打开文件，置1 
	return current_file; 	
}

//关闭文件：也是根据dir* current_dir找到其iNode，修改open_num并释放句柄 
void Close_File(os_file	*f){
	f->f_iNode->open_num = 0;
	f->f_pos = 0;
	free(f);
}

//删除普通文件 
int os_rm(string f_name, string path){
	int f_i = find_dir_no(f_name); //找到了要删除的文件目录项 
	if(f_i == DIR_NUM+1) {
		Log::w("Cannot delete the file\n");
		return 0;// 失败 
	}
	else
		return Delete_File(f_name,f_i, path); //删除 
} 

//从dir* current_dir中查找到文件在目录的位置（数组下标） 
int find_dir_no(string f_name){	 
	int i;
	for(i=0;i<DIR_NUM && (*(current_dir+i)).file_name != f_name;i++){}
	if(i == DIR_NUM){ //目录中没有该文件
		//printf("目录中没有该文件\n");
		return DIR_NUM+1;
	}
	return i; 
}

//删除文件, path为当前工作路径
int Delete_File(string f_name, int f_i, string path){
	//解析获取当前文件名
	int pos = path.rfind('/');
	string crt_name = path.substr(pos + 1);
	os_cd(path, "..");	//先回退一哈
	int crt_no = find_dir_no(crt_name); //拿到当前目录的inode编号
	iNode crt_inode = iNode_table[(*(current_dir + crt_no)).iNode_no]; //拿到当前目录的iNode
	os_cd(path, crt_name); //再回到当前目录
	//从iNode table中删除iNode信息 
	format_iNode(iNode_table + (*(current_dir+f_i)).iNode_no);
	//删除该文件目录项：直接后面的往前覆盖，然后最后一项补一下 
	int j;
	for(j=f_i; j<DIR_NUM && (*(current_dir+j)).iNode_no!=iNode_NUM+1; j++){
		(*(current_dir+j)).file_name = (*(current_dir+j+1)).file_name;
		(*(current_dir+j)).iNode_no = (*(current_dir+j+1)).iNode_no;
	}
	//(*(current_dir+j)).file_name = "#";
	//(*(current_dir+j)).iNode_no = iNode_NUM + 1;
	//buf保存当前目录
	char *buf = (char*)malloc(sizeof(dir)*DIR_NUM);
	memcpy(buf, current_dir, sizeof(dir)*DIR_NUM);


	FILE *disk_p = fopen(DEV_NAME, "rb+");
	for (int i = 0; i < FBLK_NUM; i++) {
		fseek(disk_p,crt_inode.block_address[i] * BLOCK_SIZE, SEEK_SET);
		fwrite((buf + i * BLOCK_SIZE), BLOCK_SIZE, 1, disk_p);
	} //当前目录写回文件中
	fseek(disk_p, INODE_START*BLOCK_SIZE, SEEK_SET);
	fwrite(iNode_table, sizeof(iNode_table), 1, disk_p); //iNode写回
	free(buf);
	fclose(disk_p);
	return 1;
}
