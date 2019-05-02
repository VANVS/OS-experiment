#include "shell.h"
#include <iostream>
#include <vector>
#include <stdlib.h>

Shell::Shell()
{
	_CommandTable.insert(CPair("init", &Manager::initial));
	_CommandTable.insert(CPair("cr", (func)&Manager::creat));
	_CommandTable.insert(CPair("de", (func)&Manager::destroy));
	_CommandTable.insert(CPair("req", (func)&Manager::RequestResource));
	_CommandTable.insert(CPair("rel", (func)&Manager::ReleasetResource));
	_CommandTable.insert(CPair("to", &Manager::TimeOut));
	_CommandTable.insert(CPair("list ready", &Manager::ShowRL));
	_CommandTable.insert(CPair("list block", &Manager::ShowBL));
	_CommandTable.insert(CPair("list res", &Manager::ShowRes));

}

Shell::~Shell(){

}



void Shell::Command()
{
	std::cout << "************* Welcome to WRX's shell *************\n";
	std::cout << "************** Enter \"quit\" to quit **************\n";
	std::string command;
	std::cout << "Shell >>";
	std::getline(std::cin, command);

	while (command != "quit") {
		if (command.size() == 0) {
			std::cout << "Shell >>";
			std::getline(std::cin, command);
			continue;
		}
			
		CallFunc(SplitCommand(command));
		std::cout << "Shell >>";
		std::getline(std::cin, command);
	}
	
	std::cout << "******************** Shell off ********************\n";
}

std::vector<std::string>  Shell::SplitCommand(std::string command){
	std::vector<std::string> elements;
	std::string separator = " ";
	std::string::size_type pos;

	command += separator;
	int size = command.size();
	for (int i = 0; i < size; i++) {
		pos = command.find(separator, i);
		if ((int)pos < size) {
			std::string s = command.substr(i, pos - i);
			elements.push_back(s);
			i = pos + separator.size() - 1;
		}
	}
	
	return elements;
}

void Shell::CallFunc(std::vector<std::string> elements)
{
	int size = elements.size();
	std::string type;
	if (size == 1) {
		type = *elements.begin();
		(_manager.*_CommandTable[type])();
	}
	else if (size == 2) {
		if (elements[0] == "list") {
			type = elements[0] + " " + elements[1];
			if (_CommandTable.find(type) == _CommandTable.end()) {
				std::cout << "No command match!";
				return;
			}
			(_manager.*_CommandTable[type])();
		}
		else {
			type = elements[0];
			std::string name = elements[1];
			if (_CommandTable.find(type) == _CommandTable.end()) {
				std::cout << "No command match!";
				return;
			}
			(_manager.*(func_1)_CommandTable[type])(name);
		}
	}
	else if (size == 3) {
		type = elements[0];
		std::string name = elements[1];
		int num = atoi(elements[2].c_str());
		if (_CommandTable.find(type) == _CommandTable.end()) {
			std::cout << "No command match!";
			return;
		}
		(_manager.*(func_2)_CommandTable[type])(name, num);
	}
	else if (size == 4) {
		type = elements[0];
		std::string name = elements[1];
		int num = atoi(elements[2].c_str());
		Process* p = _manager.GetProcessAddress(elements[3]);
		if (_CommandTable.find(type) == _CommandTable.end()) {
			std::cout << "No command match!";
			return;
		}
		(_manager.*(func_3)_CommandTable[type])(name, num, p);
	}
	else {
		std::cout << "Wrong command form!\n";
	}
	
}