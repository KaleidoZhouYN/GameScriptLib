#ifndef _HOOK_H
#define _HOOK_H
#include "mutex.h"
#include "shared_memory.h"
#include "screenshot.h"

class ScreenShotHook
{
public:
	ScreenShotHook(const std::string& name_, const size_t size_) :_sName(name_)
	{
		mutex = MutexSingleton::Instance(name_ + "_Mutex");
		shm = SharedMemorySingleton::Instance(name_ + "SharedMemory", size_);
	}
	bool start()
	{
		if (!mutex->open()) {
			return false;
		}
		MessageBoxA(0, "Mutex open", "OK", MB_ICONEXCLAMATION);
			
		if (!shm->open()) {
			return false; 
		}
		MessageBoxA(0, "SHM open", "OK", MB_ICONEXCLAMATION);

		return TRUE; 
	}
	~ScreenShotHook()
	{
		MutexSingleton::remove(_sName);
		SharedMemorySingleton::remove(_sName);
	}

	MutexSingleton* mutex; 
	SharedMemorySingleton* shm;

private:
	const std::string& _sName; 
};

#endif