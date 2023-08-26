#ifndef _MUTEX_H
#define _MUTEX_H
#include <windows.h>
#include <string>
#include <assert.h>
#include <map>

#include "easylogging++.h"
#define ELPP_THREAD_SAFE

class Mutex
{
public:
	Mutex(const std::string& name_) : _sName(name_), _hMutex(NULL) {};
	virtual ~Mutex()
	{
		if (_hMutex) {
			unlock();
			CloseHandle(_hMutex);
			_hMutex = NULL;
			LOG(INFO) << "Release Mutex with name: " << _sName;
		}
	}
	// ½ûÖ¹¸´ÖÆ
	Mutex& operator=(const Mutex& rhs) = delete;
	Mutex(const Mutex& rhs) = delete;

	BOOL open()
	{
		_hMutex = OpenMutex(NULL, FALSE, _sName.c_str());
		if (!_hMutex)
		{
			_hMutex = CreateMutex(NULL, FALSE, _sName.c_str());
			if (!_hMutex)
			{
				LOG(ERROR) << "Could not create a mutex with name: " << _sName;
				return FALSE;
			}
		}
		LOG(INFO) << "Create mutex with name: " << _sName;
		return TRUE;
	}
	void lock() {
		WaitForSingleObject(_hMutex, INFINITE);
	}

	DWORD try_lock(size_t time_) {
		return WaitForSingleObject(_hMutex, time_);
	}
	void unlock() {
		assert(_hMutex);
		ReleaseMutex(_hMutex);
	}

private:
	std::string _sName = "";
	HANDLE _hMutex = NULL;
};

// Make Mutex a singleton , but make Lock a resource
class MutexSingleton : public Mutex
{
public:
	static MutexSingleton* Instance(const std::string& name_)
	{
		if (_singleton.find(name_) == _singleton.end())
		{
			_singleton[name_] = new MutexSingleton(name_);
		}
		return _singleton[name_];
	};

	static void remove(const std::string& name_)
	{
		if (_singleton.find(name_) != _singleton.end())
		{
			delete _singleton[name_];
			_singleton.erase(name_);
		}
	}
protected:
	MutexSingleton(const std::string& name_) : Mutex(name_) {};
	virtual ~MutexSingleton() {};

private:
	static std::map<std::string, MutexSingleton*> _singleton;

	class GarbageCollector {
	public:
		~GarbageCollector()
		{
			for (auto it = _singleton.begin(); it != _singleton.end(); it++)
				delete it->second;
			_singleton.clear();
		}
	};
	static GarbageCollector gc;

};

/*
// Make Mutex a singleton , but make Lock a resource
class MutexSingleton
{
public:
	static MutexSingleton* Instance(const std::string& name_)
	{
		if (_singleton.find(name_) == _singleton.end())
		{
			_singleton[name_] = new MutexSingleton(name_);
		}
		return _singleton[name_];
	};

	static void remove(const std::string& name_)
	{
		if (_singleton.find(name_) != _singleton.end())
		{
			delete _singleton[name_];
			_singleton.erase(name_);
		}
	}

protected:
	MutexSingleton(const std::string& name_) : _sName(name_), _hMutex(NULL) {};
	~MutexSingleton()
	{
		if (_hMutex) {
			unlock();
			CloseHandle(_hMutex);
			_hMutex = NULL;
			LOG(INFO) << "Release Mutex with name: " << _sName; 
		}
	}

private:
	static std::map<std::string, MutexSingleton*> _singleton;

	class GarbageCollector {
	public:
		~GarbageCollector()
		{
			for (auto it = _singleton.begin(); it != _singleton.end(); it++)
				delete it->second;
			_singleton.clear();
		}
	};
	static GarbageCollector gc;

public:
	BOOL open()
	{
		_hMutex = OpenMutex(NULL, FALSE, _sName.c_str()); 
		if (!_hMutex)
		{
			_hMutex = CreateMutex(NULL, FALSE, _sName.c_str());
			if (!_hMutex)
			{
				LOG(ERROR) << "Could not create a mutex with name: " << _sName;
				return FALSE;
			}
		}
		LOG(INFO) << "Create mutex with name: " << _sName; 
		return TRUE;
	}

	void lock() {
		WaitForSingleObject(_hMutex, INFINITE);
	}

	DWORD try_lock(size_t time_) {
		return WaitForSingleObject(_hMutex, time_);
	}
	void unlock() {
		assert(_hMutex);
		ReleaseMutex(_hMutex);
	}

private:
	std::string _sName  = "";
	HANDLE _hMutex = NULL;
};
*/

class Lock
{
public:
	Lock(Mutex* mutex) { 
		_mutex = mutex;
		_mutex->lock();
		isLocked = true; 
	};
	~Lock()
	{
		// avoid duplicated release
		if (isLocked)
			unlock(); 
	}

	void unlock()
	{
		_mutex->unlock(); 
		isLocked = false; 
	}
private:
	Mutex* _mutex; 
	bool isLocked = false;
};

#endif
