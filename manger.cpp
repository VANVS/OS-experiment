#include "manager.h"
#include <cstdlib>
#include <string>
#include <iostream>


/////////////////////////////////Manger类实现////////////////////////////////////////

///////////////////////Manager类默认构造函数///////////////////////

Manager::Manager() {
	//初始资源，建立资源管理表
	for (int i = 0; i<4; i ++) {
		char num[2];
		_itoa(i + 1, num, 10);
		std::string rname = "R" + std::string(num);
		Resource R(rname, i+1);
		_ResourceList.insert(RPair(rname,R));
	}
	//运行进程销毁标志置false
	IF_RUN_PRO_DESTROY = false;

}

Manager::~Manager() {
	
}

///////////////////////创建初始进程///////////////////////

void Manager::initial() {
	Process init("init", 0);
	_ProcessList.insert(PPair("init", init));
	Process *initAddress = GetProcessAddress("init");
	_ReadyList[0].push_back(initAddress);
	_RunningProcess = initAddress;

	std::cout << "init process is running\n";
}

///////////////////////创建一个进程///////////////////////
/////////////pid：进程名，priority：优先级////////////////

void Manager::creat(std::string pid, int priority) {
	//优先级出错处理
	if (priority > 2) {
		std::cout << "priority " << priority << " don't exist. creating fallied.\n";
		return;
	}
	Process p(pid, priority);
	_ProcessList.insert(PPair(pid, p));					//奖新建进程插入到进程管理表中
	Process *pAddress = GetProcessAddress(pid);			//获取新进程在管理表中的地址
	pAddress->_parent = _RunningProcess;				//将父进程指针指向正在运行的进程
	_RunningProcess->_children.push_back(pAddress);		//将新进程的指针插入到运行进程的子进程队列中
	_ReadyList[priority].push_back(pAddress);			//将新进程的指针插入到对应就绪队列中

	scheduler();	//调度
}

///////////////////////销毁一个进程///////////////////////
///////////////////////pid：进程名////////////////////////

void Manager::destroy(std::string pid) {
	kill(pid);
	scheduler();
}

/////////////////递归销毁一个进程及其子进程////////////////
///////////////////////pid：进程名////////////////////////

void Manager::kill(std::string pid) {
	Process *p = GetProcessAddress(pid);	//找到进程名所对应的进程

	////////////如果有子进程，则递归调用消除////////////
	PList::iterator it;							
	if (!p->_children.empty()) {
		for (it = p->_children.begin(); it != p->_children.end(); it++) {
			kill((*it)->_pid);
		}
	}

	//如果销毁进程状态为就绪
	if (p->_type == READY) {
		_ReadyList[p->_priority].remove(p);	//从就绪队列中移除
	}
	//如果销毁进程状态为阻塞
	else if (p->_type == BLOCK) {
		p->_block->_WaitingList.remove(p);	//从资源R的阻塞队列中移除
	}
	//如果销毁进程状态为运行
	else {
		IF_RUN_PRO_DESTROY = true;	//运行进程销毁标志置为ture
		_ReadyList[p->_priority].remove(p);	//从就绪队列中移除
	}

	///////////////释放占有的资源///////////////
	if (!p->_PResourceList.empty()) {
		std::list<RAllocated>::iterator itr;
		for (itr = p->_PResourceList.begin(); itr != p->_PResourceList.end(); itr++) {
			ReleasetResource(itr->first->_rid, itr->second, p);		//调用释放资源函数释放占有资源
			if (p->_PResourceList.empty())
				break;
		}
	}
	
	//从进程管理表中删除进程所在项
	_ProcessList.erase(pid);
}

/////////////////////运行进程请求资源/////////////////////
///////////////rid：资源名，request：申请量///////////////

void Manager::RequestResource(std::string rid, int request){
	Resource *r = GetResourceAddress(rid);			//找到资源名所对应的资源r

	//如果申请量小于资源可提供量，则分配
	if (r->_avaliable >= request) {
		r->_avaliable -= request;
		_RunningProcess->_PResourceList.push_back(RAllocated(r, request));

		std::cout << "process " << _RunningProcess->_pid << " requests " << request << ' ' << rid << std::endl;
	}
	//否则阻塞
	else {
		//申请是否超过资源的总量，即初始量，有则报错
		if (request > r->_initial) {
			std::cout << "Cannot request resource more than initial number.\n";
			return;
		}
		_RunningProcess->_type = BLOCK;					//运行进程置为阻塞态
		_RunningProcess->_block = r;					//阻塞资源置为r
		_RunningProcess->_blocknum = request;			//阻塞资源数置为request
		_ReadyList[_RunningProcess->_priority].remove(_RunningProcess);	//将运行进程从就绪队列中移除
		r->_WaitingList.push_back(_RunningProcess);						//将运行进程插入资源r的阻塞等待序列中
		scheduler();	//调度
	}
	
}

//////////////////////////////进程p释放资源//////////////////////////////
//////////rid：资源名，release：申请量，p：释放资源的进程地址////////////

void Manager::ReleasetResource(std::string rid, int release, Process *p){
	Resource *r = GetResourceAddress(rid);		//找到资源名所对应的资源r

	////////////查找是否有满足条件的资源////////////
	std::list<RAllocated>::iterator it;
	it = std::find(p->_PResourceList.begin(), p->_PResourceList.end(), RAllocated(r, release));
	//若没有则报错直接退出
	if (it == p->_PResourceList.end() || (*it).second != release) {
		std::cout << p->_pid << " donnot has resource (" << rid << ',' << release << ")\n";
		return;
	}
	
	r->_avaliable += release;				//更新资源r可提供量
	p->_PResourceList.remove(*it);			//从进程p的资源列表中移除资源r
	std::cout << "Release " << rid << '.';	//打印释放信息

	Process *WakePossible = r->_WaitingList.front();		//阻塞队列中的第一个进程，可能会被唤醒
	//如果阻塞队列不空，并且队首进程阻塞资源数小于资源可提供数
	while (!r->_WaitingList.empty() && r->_avaliable >= WakePossible->_blocknum) {		
		std::cout << " wake up process " << WakePossible->_pid << '\n';					//打印唤醒信息
		r->_avaliable -= WakePossible->_blocknum;										//更新资源可提供数
		r->_WaitingList.pop_front();													//将被唤醒进程移除
		WakePossible->_type = READY;													//将被唤醒进程的状态置为就绪态
		WakePossible->_block = NULL;													//将被唤醒进程的阻塞资源置为空
		WakePossible->_PResourceList.push_back(RAllocated(r, WakePossible->_blocknum));	//将资源r和占有数插入被唤醒进程的资源列表
		WakePossible->_blocknum = 0;													//将被唤醒进程的阻塞数置为0
		_ReadyList[WakePossible->_priority].push_back(WakePossible);						//将被唤醒进程插入对应的就绪队列
	}
	scheduler();	//调度
}

///////////////////////////////调度函数///////////////////////////////

void Manager::scheduler(){
	//寻找最高优先级进程
	Process * HighestProcess = GetProcessAddress("init");
	PMap::iterator it;
	for (it = _ProcessList.begin(); it != _ProcessList.end(); it++) {
		if (it->second._priority > HighestProcess->_priority)
			HighestProcess = &(it->second);
	}

	std::string info = " \n";		//待打印的补充信息

	//资源释放后唤醒进程的优先级高于当前进程优先级
	if (_RunningProcess->_priority < HighestProcess->_priority) {
		_RunningProcess = HighestProcess;
	}
	//请求资源使得当前运行进程阻塞
	else if (_RunningProcess->_type == BLOCK) {
		info = "process " + _RunningProcess->_pid + " is blocked\n";		//存储阻塞进程信息
		_RunningProcess = *_ReadyList[_RunningProcess->_priority].begin();	//运行进程更新为就绪队列队首进程
	}
	//时钟中断使得当前运行进程变成就绪
	else if (_RunningProcess->_type == READY) {
		//如果当前就绪队列只有不多于两个的进程就不打印就绪进程信息了
		if (_ReadyList[_RunningProcess->_priority].size() > 2)
			info = "process " + _RunningProcess->_pid + " is ready\n";		//存储阻塞进程信息

		_RunningProcess = *_ReadyList[_RunningProcess->_priority].begin();	//运行进程更新为就绪队列队首进程
	}
	//运行进程销毁
	else if (IF_RUN_PRO_DESTROY) {
		_RunningProcess = HighestProcess;
	}
	else {
		
	}
	_RunningProcess->_type = RUN;			//新运行进程状态置为运行态
	std::cout << "process " << _RunningProcess->_pid << " is running. " << info;	//打印目前运行进程和阻塞或者就绪进程
}

//////////////////////////////模拟时钟中断///////////////////////////////

void Manager::TimeOut(){
	
	if (_ReadyList[_RunningProcess->_priority].empty()) {				//判断当前就绪队列是否为空
		return;
	}

	PList & RListCurrent = _ReadyList[_RunningProcess->_priority];	//找到当前运行进程所在就绪队列
	RListCurrent.pop_front();										//将当前运行进程从就绪队列头部移除
	RListCurrent.push_back(_RunningProcess);						//将当前运行进程从就绪队列尾部插入
	_RunningProcess->_type = READY;									//将当前运行进程状态改为“就绪”
	scheduler();
}

//////////////////////////////显示就绪队列///////////////////////////////

void Manager::ShowRL()
{
	for (int i = 2; i >= 0; i--) {
		std::cout << i << ": ";
		PList::iterator itr;
		for (itr = _ReadyList[i].begin(); itr != _ReadyList[i].end(); itr++) {
			std::cout << (*itr)->_pid << '-';
		}
		std::cout << '\b' << ' ' << std::endl;
	}
}

//////////////////////////////显示阻塞队列///////////////////////////////

void Manager::ShowBL()
{
	RMap::iterator itm;
	for (itm = _ResourceList.begin(); itm != _ResourceList.end(); itm++) {
		std::cout << itm->second._rid << " ";
		PList::iterator itr;
		for (itr = itm->second._WaitingList.begin(); itr != itm->second._WaitingList.end(); itr++) {
			std::cout << (*itr)->_pid << '-';
		}
		std::cout << '\b' << ' ' << std::endl;
	}
}

/////////////////////////显示每个资源的可提供量//////////////////////////

void Manager::ShowRes()
{
	RMap::iterator itm;
	for (itm = _ResourceList.begin(); itm != _ResourceList.end(); itm++) {
		std::cout << itm->second._rid << " " << itm->second._avaliable << std::endl;
	}
}

////////////////////////获取进程名对应进程在进程管理表中的地址////////////////////
Process * Manager::GetProcessAddress(std::string name){
	return &_ProcessList.find(name)->second;
}

////////////////////////获取资源名对应资源在资源管理表中的地址////////////////////
Resource * Manager::GetResourceAddress(std::string name)
{
	return &_ResourceList.find(name)->second;;
}

