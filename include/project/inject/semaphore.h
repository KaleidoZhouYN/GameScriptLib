#ifndef _SEMAPHORE_H
#define _SEMAPHORE_H
#include <windows.h>
#include <string>
#include <assert.h> 

#include "easylogging++.h"
#define ELPP_THREAD_SAFE

class Semaphore
{
public:
	Semaphore(const std::string& name_, size_t size_) : _sName(name_), _hSemaphore(NULL),_iMaxCount(size_) {};
	virtual ~Semaphore()
	{
		if (_hSemaphore) {
			//unlock();
			CloseHandle(_hSemaphore);
			_hSemaphore = NULL;
			LOG(INFO) << "Release Semaphore with name: " << _sName;
		}
	}
	// ½ûÖ¹¸´ÖÆ
	Semaphore& operator=(const Semaphore& rhs) = delete;
	Semaphore(const Semaphore& rhs) = delete;

	BOOL open()
	{
		_hSemaphore = CreateSemaphore(NULL, _iMaxCount, _iMaxCount,_sName.c_str());
		if (!_hSemaphore)
		{
			LOG(ERROR) << "Could not create a Semaphore with name: " << _sName;
			return FALSE;
		}
		LOG(INFO) << "Create Semaphore with name: " << _sName;
		return TRUE;
	}
	void wait() {
		WaitForSingleObject(_hSemaphore, INFINITE);
	}
	void post() {
		ReleaseSemaphore(_hSemaphore,1, NULL);
	}

private:
	std::string _sName = "";
	HANDLE _hSemaphore = NULL;
	size_t _iMaxCount = 0; 
};

#endif