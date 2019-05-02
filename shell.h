#pragma once
#ifndef SHELL_H_
#define SHELL_H_

#include "manager.h"
#include <map>
#include <string>

typedef void (Manager::*func)();
typedef void (Manager::*func_1)(std::string); 
typedef void (Manager::*func_2)(std::string, int);
typedef void (Manager::*func_3)(std::string, int, Process*);

typedef std::map<std::string, func> CTable;
typedef std::pair<std::string, func> CPair;

class Shell {
	
private:
	CTable _CommandTable;
public:
	Manager _manager;

	Shell();
	~Shell();

	void Command();
	std::vector<std::string>  SplitCommand(std::string);
	void CallFunc(std::vector<std::string>);

};
#endif // !SHELL_H_
