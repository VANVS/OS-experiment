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

typedef std::list<Process*> PList;		//�洢������ָ�������
typedef std::list<Resource*> RList;		//�洢��Դ��ָ�������

///////////////////////////////////////��Դ��/////////////////////////////////////////////
class Resource {
public:
	std::string _rid;		//��Դ����
	int _initial;			//��Դ��ʼ��
	int _avaliable;			//��Դ���ṩ��
	PList _WaitingList;		//�ȴ�����Դ���������̵���������

	Resource();				
	Resource(std::string rid, int initial);
	~Resource();
};

///////////////////////////////////////������/////////////////////////////////////////////

typedef std::pair<Resource*, int> RAllocated;	//���ڴ洢��ռ��Դ��ָ�����Դ�����Ķ�

class Process {
public:
	std::string _pid;						//��������
	std::list<RAllocated> _PResourceList;	//�洢������ռ��Դ��ָ�����Դ����

	int _type;			//����״̬��run, ready or block
	Resource * _block;	//ָ����������ȴ�����Դ
	int _blocknum;		//�����ȴ�����Դ��

	Process* _parent;	//ָ�򸸽���
	PList _children;	//�洢ָ���ӽ���ָ�������

	int _priority;		//���̵����ȼ�

	Process();
	Process(std::string pid, int priority, int type = READY);
	~Process();
};


typedef std::map<std::string, Process> PMap;	//�洢���̵Ĺ�ϣ�����м�ֵΪ���̵�����(ID)
typedef std::map<std::string, Resource> RMap;	//�洢��Դ�Ĺ�ϣ�����м�ֵΪ���̵�����(ID)
typedef std::pair<std::string, Process> PPair;	//���̹�ϣ���һ����ڹ�ϣ��Ĳ���
typedef std::pair<std::string, Resource> RPair;	//��Դ��ϣ���һ����ڹ�ϣ��Ĳ���

class Manager {
private:
	enum {ListSize = 3};			
	PList _ReadyList[ListSize];		//�������У��䱾����3�������Ľ���ָ������
	PMap _ProcessList;				//���̹�����䱾���Ǽ�ֵΪ����ID�Ĺ�ϣ��
	RMap _ResourceList;				//���̹�����䱾���Ǽ�ֵΪ����ID�Ĺ�ϣ��
	Process * _RunningProcess;		//point to the Process running now
	bool IF_RUN_PRO_DESTROY;	//��־���ж����н����Ƿ�����
public:
	Manager();
	~Manager();
	void initial();														//������ʼ����
	void creat(std::string pid, int priority);							//����һ�����̣����ṩ�����������ȼ�
	void destroy(std::string pid);										//����һ�����̣����ṩ������
	void kill(std::string pid);											//�ݹ�����һ�����̼����ӽ��̣����ṩ������
	void RequestResource(std::string rid, int request);					//���н���������Դ�����ṩ��Դ������������
	void ReleasetResource(std::string rid, int release, Process* p );	//����p�ͷ���Դ�����ṩ��Դ�����ͷ�����������ָ��
	void scheduler();		//���Ⱥ���
	void TimeOut();			//ģ��ʱ���ж�
	void ShowRL();			//��ʾ��������
	void ShowBL();			//��ʾÿ����Դ����������
	void ShowRes();			//��ʾÿ����Դ�Ŀ��ṩ��
	Process* GetProcessAddress(std::string);	//��ȡ��������Ӧ�����ڽ��̹�����еĵ�ַ
	Resource* GetResourceAddress(std::string);	//��ȡ��Դ����Ӧ��Դ����Դ������еĵ�ַ
};
#endif // !MANAGER_H_
