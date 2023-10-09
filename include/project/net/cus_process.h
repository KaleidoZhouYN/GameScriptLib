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
template<typename T> std::shared_ptr<BaseProcess> createSharedPtrProcess() { return std::make_shared<T>(); };

#endif