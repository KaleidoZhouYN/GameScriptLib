#ifndef _HOOK_H
#define _HOOK_H
#include "mutex.h"
#include "screenshot.h"

class ScreenShotHook
{
public:
	ScreenShotHook(const std::string& name_, const size_t size_) :_sName(name_)
	{
		mutex = MutexSingleton::Instance(name_ + "_Mutex");
		shm = SharedMemorySingleton::Instance(name_ + "SharedMemory");
	}
	bool start()
	{
		if (!mutex->open()) {
			return False;
		}
			
		if (!shm->open()) {
			return False; 
		}

		return TRUE; 
	}
	~ScreenShotHook()
	{
		MutexSingleton::remove(_sName);
		SharedMemorySingleton::remove(_sName);
	}
private:
	MutexSingleton* mutex; 
	SharedMemorySingleton* shm; 
	const std::string& _sName; 
};

#endif