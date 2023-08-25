#ifndef _MUTEX_H
#define _MUTEX_H
#include <windows.h>
#include <string>
#include <assert.h>
#include <map>
#include <unordered_map>

// because Mutex need to release while exception happen
// it's no need to make it a singleton 
// but just to make it a shared variabel
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
			_hMutex = CreateMutex(NULL, FALSE, _sName.c_str());
			if (!_hMutex)
				return FALSE;
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

class Lock
{
public:
	Lock(MutexSingleton* mutex) { 
		_mutex = mutex;
		_mutex->lock();
		isLocked = true; 
	};
	~Lock()
	{
		if (isLocked)
			unlock(); 
	}

	void unlock()
	{
		_mutex->unlock(); 
		isLocked = false; 
	}
private:
	MutexSingleton* _mutex; 
	bool isLocked = false;
};

#endif


/* CreateMutex���ص�handle�ǲ�ͬ�ģ����Բ�����shared handle�����
 CloseHandle�����Mutex��������������൱���Դ�smart pointer
 ��Ϊhandle�ǲ�һ���ģ�����Mutex���ܸ���
 ��ΪCreateMutex��OpenMutexЧ�ʲ�һ����������Ҫһ��ȫ�ֱ�����CreateMutex
 �ֲ�����ʹ��OpenMutex
 
class Mutex
{
public:
	Mutex() = default;
	Mutex(const std::string& name_) : _sName(name_),_hMutex(NULL) { };

	// ��ֹ����
	Mutex& operator=(const Mutex& rhs) = delete; 
	Mutex(const Mutex& rhs) = delete; 

	bool open()
	{
		if (_sName == "")
			return FALSE;
		if (_hMutex != NULL)
			return TRUE; 
		// call singleton Instance
		if (!MutexSingleton::Instance(_sName))
			return FALSE;
		_hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, _sName.c_str());
		if (!_hMutex)
			return FALSE;
		return TRUE;
	}
	~Mutex() {
		if (_hMutex) {
			unlock(); 
			_hMutex = NULL; 
			CloseHandle(_hMutex);
		}
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

*/
