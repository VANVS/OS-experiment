#pragma once
#ifndef MANAGER_H_
#define MANAGER_H_

#include <list>
#include <string>
#include <vector>
#include <map>

#define RUN 1
#define READY 0
#define BLOCK -1

class Resource;
class Process;
class Manager;

typedef std::list<Process*> PList;		//存储进程类指针的链表
typedef std::list<Resource*> RList;		//存储资源类指针的链表

///////////////////////////////////////资源类/////////////////////////////////////////////
class Resource {
public:
	std::string _rid;		//资源名称
	int _initial;			//资源初始量
	int _avaliable;			//资源可提供量
	PList _WaitingList;		//等待本资源的阻塞进程的阻塞队列

	Resource();				
	Resource(std::string rid, int initial);
	~Resource();
};

///////////////////////////////////////进程类/////////////////////////////////////////////

typedef std::pair<Resource*, int> RAllocated;	//用于存储所占资源的指针和资源数量的对

class Process {
public:
	std::string _pid;						//进程名称
	std::list<RAllocated> _PResourceList;	//存储进程所占资源的指针和资源数量

	int _type;			//进程状态：run, ready or block
	Resource * _block;	//指向进程阻塞等待的资源
	int _blocknum;		//阻塞等待的资源数

	Process* _parent;	//指向父进程
	PList _children;	//存储指向子进程指针的链表

	int _priority;		//进程的优先级

	Process();
	Process(std::string pid, int priority, int type = READY);
	~Process();
};


typedef std::map<std::string, Process> PMap;	//存储进程的哈希表，其中键值为进程的名称(ID)
typedef std::map<std::string, Resource> RMap;	//存储资源的哈希表，其中键值为进程的名称(ID)
typedef std::pair<std::string, Process> PPair;	//进程哈希表的一项，用于哈希表的插入
typedef std::pair<std::string, Resource> RPair;	//资源哈希表的一项，用于哈希表的插入

class Manager {
private:
	enum {ListSize = 3};			
	PList _ReadyList[ListSize];		//就绪队列，其本质是3个连续的进程指针链表
	PMap _ProcessList;				//进程管理表，其本质是键值为进程ID的哈希表
	RMap _ResourceList;				//进程管理表，其本质是键值为进程ID的哈希表
	Process * _RunningProcess;		//point to the Process running now
	bool IF_RUN_PRO_DESTROY;	//标志，判断运行进程是否被销毁
public:
	Manager();
	~Manager();
	void initial();														//创建初始进程
	void creat(std::string pid, int priority);							//创建一个进程，需提供进程名和优先级
	void destroy(std::string pid);										//销毁一个进程，需提供进程名
	void kill(std::string pid);											//递归销毁一个进程及其子进程，需提供进程名
	void RequestResource(std::string rid, int request);					//运行进程请求资源，需提供资源名和需求数量
	void ReleasetResource(std::string rid, int release, Process* p );	//进程p释放资源，需提供资源名，释放数量及进程指针
	void scheduler();		//调度函数
	void TimeOut();			//模拟时钟中断
	void ShowRL();			//显示就绪队列
	void ShowBL();			//显示每个资源的阻塞队列
	void ShowRes();			//显示每个资源的可提供量
	Process* GetProcessAddress(std::string);	//获取进程名对应进程在进程管理表中的地址
	Resource* GetResourceAddress(std::string);	//获取资源名对应资源在资源管理表中的地址
};
#endif // !MANAGER_H_
