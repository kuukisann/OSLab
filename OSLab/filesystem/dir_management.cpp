#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_management.h"
#include "../log/Log.h"
#include <iostream>
#include <string>
#include <vector>
using namespace std;

//定义三个全局变量
iNode iNode_table[iNode_NUM];
dir root_dir[DIR_NUM];
dir* current_dir;

//dir数组初始化函数
void init_dir(dir blankdir[]){
	int i;
	for(i=0;i<DIR_NUM;i++){//将root所有文件目录项初始化为空闲状态 
		blankdir[i].file_name = "#";
		blankdir[i].iNode_no = iNode_NUM + 1;
	} 
}

//init_dir(root_dir); //将根目录初始化 

//删除所有空目录
bool os_rmdir(string dir_name, string path){
	int f_i = find_dir_no(dir_name);//找到了要删除的文件目录项 
	if(f_i == DIR_NUM+1){
		Log::w("Find the wrong directory.\n");
		return 0; 
	}
	//////////////////	
	if (iNode_table[(*(current_dir + f_i)).iNode_no].i_mode == 1) {
		Log::w("An directory file is requested.\n");
		return 0; //普通文件，错误
	}
	dir* f_dir = (dir*)malloc(sizeof(dir)*DIR_NUM);
	get_dir(f_dir, &iNode_table[(*(current_dir+f_i)).iNode_no]);//传入iNode结点，返回文件的dir数组
	int i;
	for(i = 0; i < DIR_NUM && f_dir[i].iNode_no == iNode_NUM + 1; i++){} //空的
	//////////////////
	if(i == DIR_NUM){
		return Delete_File(dir_name,f_i,path);			
	} 
	else{
		Log::w("The directory is not empty\n");
		return 0;
	}

	free(f_dir);
	return 1;
}

//列出目录下所有文件 设置默认参数是当前目录下的ls 
vector<string> os_ls() {
	vector<string> fnames;
	int i;
	for (i = 0; i<DIR_NUM && current_dir[i].file_name != "#"; i++) { //根据文件类型做区分（目录文件还是普通文件） 
		fnames.push_back(current_dir[i].file_name);
	}
	return fnames;
}

//切换目录 
bool os_cd(string &currentpath, string newpath) {//currentpath:绝对路径，newpath:绝对or相对 
	int tmp_no = -1;
	if (newpath[0] == '/') {//绝对路径 
		dir* temp = analyse_Path(newpath, &tmp_no);
		if (temp == NULL){
			Log::i("absolute path: Cannot find the directory.\n");
			return 0;
		}
			
		current_dir = temp;
		currentpath = newpath; //修改当前路径 
	}
	else {
		if (newpath == "..") { //返回上级目录 /a/b/c ->/a/b
			int pos = currentpath.rfind('/');
			if (pos != 0) {//根目录不能向上切换，无操作 
				dir* temp = analyse_Path(currentpath.substr(0, pos), &tmp_no);
				if (temp == NULL){
					Log::w("..: Cannot find the directory.\n");
					return 0;
				}
					
				current_dir = temp;
				currentpath = currentpath.substr(0, pos); //修改当前路径 
			}
		}
		else {//相对路径 /a/b/c -> /a/b/c / d   /a/b/c -> /a/b/c /d/e
			int f_i;
			if (newpath.rfind('/') != string::npos) { //相对路径大于一层 
				//newpath = '/' + newpath; //前面补上'/'
				dir* temp = analyse_Path(currentpath+'/'+newpath, &tmp_no);
				if (temp == NULL)
					Log::w("relative path: Cannot find the directory.\n");
					return 0;
				current_dir = temp;
			}
			else {
				f_i = find_dir_no(newpath); //找到current_dir中该文件的下标号
				if (f_i == DIR_NUM + 1) {
					Log::w("cd: Cannot find the directory in the current directory.\n");
					return 0;
				}
				dir* d_dir = (dir*)malloc(sizeof(dir)*DIR_NUM);
				get_dir(d_dir, &iNode_table[current_dir[f_i].iNode_no]);
				current_dir = d_dir;
			}
			currentpath = currentpath + '/' + newpath; //修改当前路径 
		}
	}
	return 1;
}
