#include "manager.h"
#include <cstdlib>
#include <string>
#include <iostream>


/////////////////////////////////Manger��ʵ��////////////////////////////////////////

///////////////////////Manager��Ĭ�Ϲ��캯��///////////////////////

Manager::Manager() {
	//��ʼ��Դ��������Դ�����
	for (int i = 0; i<4; i ++) {
		char num[2];
		_itoa(i + 1, num, 10);
		std::string rname = "R" + std::string(num);
		Resource R(rname, i+1);
		_ResourceList.insert(RPair(rname,R));
	}
	//���н������ٱ�־��false
	IF_RUN_PRO_DESTROY = false;

}

Manager::~Manager() {
	
}

///////////////////////������ʼ����///////////////////////

void Manager::initial() {
	Process init("init", 0);
	_ProcessList.insert(PPair("init", init));
	Process *initAddress = GetProcessAddress("init");
	_ReadyList[0].push_back(initAddress);
	_RunningProcess = initAddress;

	std::cout << "init process is running\n";
}

///////////////////////����һ������///////////////////////
/////////////pid����������priority�����ȼ�////////////////

void Manager::creat(std::string pid, int priority) {
	//���ȼ�������
	if (priority > 2) {
		std::cout << "priority " << priority << " don't exist. creating fallied.\n";
		return;
	}
	Process p(pid, priority);
	_ProcessList.insert(PPair(pid, p));					//���½����̲��뵽���̹������
	Process *pAddress = GetProcessAddress(pid);			//��ȡ�½����ڹ�����еĵ�ַ
	pAddress->_parent = _RunningProcess;				//��������ָ��ָ���������еĽ���
	_RunningProcess->_children.push_back(pAddress);		//���½��̵�ָ����뵽���н��̵��ӽ��̶�����
	_ReadyList[priority].push_back(pAddress);			//���½��̵�ָ����뵽��Ӧ����������

	scheduler();	//����
}

///////////////////////����һ������///////////////////////
///////////////////////pid��������////////////////////////

void Manager::destroy(std::string pid) {
	kill(pid);
	scheduler();
}

/////////////////�ݹ�����һ�����̼����ӽ���////////////////
///////////////////////pid��������////////////////////////

void Manager::kill(std::string pid) {
	Process *p = GetProcessAddress(pid);	//�ҵ�����������Ӧ�Ľ���

	////////////������ӽ��̣���ݹ��������////////////
	PList::iterator it;							
	if (!p->_children.empty()) {
		for (it = p->_children.begin(); it != p->_children.end(); it++) {
			kill((*it)->_pid);
		}
	}

	//������ٽ���״̬Ϊ����
	if (p->_type == READY) {
		_ReadyList[p->_priority].remove(p);	//�Ӿ����������Ƴ�
	}
	//������ٽ���״̬Ϊ����
	else if (p->_type == BLOCK) {
		p->_block->_WaitingList.remove(p);	//����ԴR�������������Ƴ�
	}
	//������ٽ���״̬Ϊ����
	else {
		IF_RUN_PRO_DESTROY = true;	//���н������ٱ�־��Ϊture
		_ReadyList[p->_priority].remove(p);	//�Ӿ����������Ƴ�
	}

	///////////////�ͷ�ռ�е���Դ///////////////
	if (!p->_PResourceList.empty()) {
		std::list<RAllocated>::iterator itr;
		for (itr = p->_PResourceList.begin(); itr != p->_PResourceList.end(); itr++) {
			ReleasetResource(itr->first->_rid, itr->second, p);		//�����ͷ���Դ�����ͷ�ռ����Դ
			if (p->_PResourceList.empty())
				break;
		}
	}
	
	//�ӽ��̹������ɾ������������
	_ProcessList.erase(pid);
}

/////////////////////���н���������Դ/////////////////////
///////////////rid����Դ����request��������///////////////

void Manager::RequestResource(std::string rid, int request){
	Resource *r = GetResourceAddress(rid);			//�ҵ���Դ������Ӧ����Դr

	//���������С����Դ���ṩ���������
	if (r->_avaliable >= request) {
		r->_avaliable -= request;
		_RunningProcess->_PResourceList.push_back(RAllocated(r, request));

		std::cout << "process " << _RunningProcess->_pid << " requests " << request << ' ' << rid << std::endl;
	}
	//��������
	else {
		//�����Ƿ񳬹���Դ������������ʼ�������򱨴�
		if (request > r->_initial) {
			std::cout << "Cannot request resource more than initial number.\n";
			return;
		}
		_RunningProcess->_type = BLOCK;					//���н�����Ϊ����̬
		_RunningProcess->_block = r;					//������Դ��Ϊr
		_RunningProcess->_blocknum = request;			//������Դ����Ϊrequest
		_ReadyList[_RunningProcess->_priority].remove(_RunningProcess);	//�����н��̴Ӿ����������Ƴ�
		r->_WaitingList.push_back(_RunningProcess);						//�����н��̲�����Դr�������ȴ�������
		scheduler();	//����
	}
	
}

//////////////////////////////����p�ͷ���Դ//////////////////////////////
//////////rid����Դ����release����������p���ͷ���Դ�Ľ��̵�ַ////////////

void Manager::ReleasetResource(std::string rid, int release, Process *p){
	Resource *r = GetResourceAddress(rid);		//�ҵ���Դ������Ӧ����Դr

	////////////�����Ƿ���������������Դ////////////
	std::list<RAllocated>::iterator it;
	it = std::find(p->_PResourceList.begin(), p->_PResourceList.end(), RAllocated(r, release));
	//��û���򱨴�ֱ���˳�
	if (it == p->_PResourceList.end() || (*it).second != release) {
		std::cout << p->_pid << " donnot has resource (" << rid << ',' << release << ")\n";
		return;
	}
	
	r->_avaliable += release;				//������Դr���ṩ��
	p->_PResourceList.remove(*it);			//�ӽ���p����Դ�б����Ƴ���Դr
	std::cout << "Release " << rid << '.';	//��ӡ�ͷ���Ϣ

	Process *WakePossible = r->_WaitingList.front();		//���������еĵ�һ�����̣����ܻᱻ����
	//����������в��գ����Ҷ��׽���������Դ��С����Դ���ṩ��
	while (!r->_WaitingList.empty() && r->_avaliable >= WakePossible->_blocknum) {		
		std::cout << " wake up process " << WakePossible->_pid << '\n';					//��ӡ������Ϣ
		r->_avaliable -= WakePossible->_blocknum;										//������Դ���ṩ��
		r->_WaitingList.pop_front();													//�������ѽ����Ƴ�
		WakePossible->_type = READY;													//�������ѽ��̵�״̬��Ϊ����̬
		WakePossible->_block = NULL;													//�������ѽ��̵�������Դ��Ϊ��
		WakePossible->_PResourceList.push_back(RAllocated(r, WakePossible->_blocknum));	//����Դr��ռ�������뱻���ѽ��̵���Դ�б�
		WakePossible->_blocknum = 0;													//�������ѽ��̵���������Ϊ0
		_ReadyList[WakePossible->_priority].push_back(WakePossible);						//�������ѽ��̲����Ӧ�ľ�������
	}
	scheduler();	//����
}

///////////////////////////////���Ⱥ���///////////////////////////////

void Manager::scheduler(){
	//Ѱ��������ȼ�����
	Process * HighestProcess = GetProcessAddress("init");
	PMap::iterator it;
	for (it = _ProcessList.begin(); it != _ProcessList.end(); it++) {
		if (it->second._priority > HighestProcess->_priority)
			HighestProcess = &(it->second);
	}

	std::string info = " \n";		//����ӡ�Ĳ�����Ϣ

	//��Դ�ͷź��ѽ��̵����ȼ����ڵ�ǰ�������ȼ�
	if (_RunningProcess->_priority < HighestProcess->_priority) {
		_RunningProcess = HighestProcess;
	}
	//������Դʹ�õ�ǰ���н�������
	else if (_RunningProcess->_type == BLOCK) {
		info = "process " + _RunningProcess->_pid + " is blocked\n";		//�洢����������Ϣ
		_RunningProcess = *_ReadyList[_RunningProcess->_priority].begin();	//���н��̸���Ϊ�������ж��׽���
	}
	//ʱ���ж�ʹ�õ�ǰ���н��̱�ɾ���
	else if (_RunningProcess->_type == READY) {
		//�����ǰ��������ֻ�в����������Ľ��̾Ͳ���ӡ����������Ϣ��
		if (_ReadyList[_RunningProcess->_priority].size() > 2)
			info = "process " + _RunningProcess->_pid + " is ready\n";		//�洢����������Ϣ

		_RunningProcess = *_ReadyList[_RunningProcess->_priority].begin();	//���н��̸���Ϊ�������ж��׽���
	}
	//���н�������
	else if (IF_RUN_PRO_DESTROY) {
		_RunningProcess = HighestProcess;
	}
	else {
		
	}
	_RunningProcess->_type = RUN;			//�����н���״̬��Ϊ����̬
	std::cout << "process " << _RunningProcess->_pid << " is running. " << info;	//��ӡĿǰ���н��̺��������߾�������
}

//////////////////////////////ģ��ʱ���ж�///////////////////////////////

void Manager::TimeOut(){
	
	if (_ReadyList[_RunningProcess->_priority].empty()) {				//�жϵ�ǰ���������Ƿ�Ϊ��
		return;
	}

	PList & RListCurrent = _ReadyList[_RunningProcess->_priority];	//�ҵ���ǰ���н������ھ�������
	RListCurrent.pop_front();										//����ǰ���н��̴Ӿ�������ͷ���Ƴ�
	RListCurrent.push_back(_RunningProcess);						//����ǰ���н��̴Ӿ�������β������
	_RunningProcess->_type = READY;									//����ǰ���н���״̬��Ϊ��������
	scheduler();
}

//////////////////////////////��ʾ��������///////////////////////////////

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

//////////////////////////////��ʾ��������///////////////////////////////

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

/////////////////////////��ʾÿ����Դ�Ŀ��ṩ��//////////////////////////

void Manager::ShowRes()
{
	RMap::iterator itm;
	for (itm = _ResourceList.begin(); itm != _ResourceList.end(); itm++) {
		std::cout << itm->second._rid << " " << itm->second._avaliable << std::endl;
	}
}

////////////////////////��ȡ��������Ӧ�����ڽ��̹�����еĵ�ַ////////////////////
Process * Manager::GetProcessAddress(std::string name){
	return &_ProcessList.find(name)->second;
}

////////////////////////��ȡ��Դ����Ӧ��Դ����Դ������еĵ�ַ////////////////////
Resource * Manager::GetResourceAddress(std::string name)
{
	return &_ResourceList.find(name)->second;;
}

