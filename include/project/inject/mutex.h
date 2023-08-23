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

std::map<std::string, MutexSingleton*> MutexSingleton::_singleton = {};
MutexSingleton::GarbageCollector MutexSingleton::gc;

class Lock
{
public:
	Lock(MutexSingleton* mutex) { 
		_mutex = mutex;
		_mutex->lock();
	};
	~Lock()
	{
		unlock(); 
	}

	void unlock()
	{
		_mutex->unlock(); 
	}
private:
	MutexSingleton* _mutex; 
};


/* CreateMutex返回的handle是不同的，所以不存在shared handle的情况
 CloseHandle会减少Mutex对象的引用数，相当于自带smart pointer
 因为handle是不一样的，所以Mutex不能复制
 因为CreateMutex和OpenMutex效率不一样，所以需要一个全局变量来CreateMutex
 局部变量使用OpenMutex
 
class Mutex
{
public:
	Mutex() = default;
	Mutex(const std::string& name_) : _sName(name_),_hMutex(NULL) { };

	// 禁止复制
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
