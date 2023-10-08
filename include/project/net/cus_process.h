#ifndef _PROCESS_H
#define _PROCESS_H
#include <windows.h>


class BaseProcess
{
public:
	BaseProcess() {}
	virtual ~BaseProcess() {}
	virtual void LoadConfig() {};
	virtual void doProcess(LPARAM src, LPARAM dst) {};
};


template<typename T> BaseProcess* createProcess() { return new T(); };

#endif