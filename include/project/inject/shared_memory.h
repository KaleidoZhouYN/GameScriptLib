#ifndef _SHARED_MEMORY_H
#define _SHARED_MEMORY_H

#include <string>
#include <map>
#include <memory>
#include <windows.h>

#include "easylogging++.h"
#define ELPP_THREAD_SAFE  

// 使用的时候需要用shared_ptr
class SharedMemory
{
public:
	SharedMemory(const std::string& name_,const size_t size_) :_bShmData(nullptr), _sShmName(name_), _iMaxShmSize(size_), _hMapFile(NULL) { };
	virtual ~SharedMemory() {
		close(); 
	}

	SharedMemory(const SharedMemory& rhs) = delete;
	SharedMemory& operator=(const SharedMemory& rhs) = delete;


	bool open()
	{
		_hMapFile = CreateFileMapping(
			INVALID_HANDLE_VALUE,
			NULL,
			PAGE_READWRITE,
			0,
			_iMaxShmSize,
			_sShmName.c_str());

		if (_hMapFile == NULL) {
			// add log here
			LOG(ERROR) << " Could not create file mapping object with name: " << _sShmName;
			//MessageBoxA(0, "Could not create file mapping object", "Fail", MB_ICONEXCLAMATION);
			return FALSE;
		}

		_bShmData = static_cast<void*>(MapViewOfFile(_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, _iMaxShmSize));

		if (_bShmData == nullptr) {
			// add log here
			LOG(ERROR) << "Could not map view of file with name: " << _sShmName;
			//MessageBoxA(0, "Could not map view of file.", "Fail", MB_ICONEXCLAMATION);
			return FALSE;
		}

		LOG(INFO) << "Shared Memory created with name: " << _sShmName;

		return TRUE;
	}
	void close()
	{
		if (_hMapFile)
		{
			UnmapViewOfFile(_hMapFile); // 释放虚拟内存
			CloseHandle(_hMapFile);
			_hMapFile = NULL; 
			LOG(INFO) << "Shared Memory has benn released with name: " << _sShmName;
		}
	}
	template<typename T>
	T* data() {
		return static_cast<T*>(_bShmData);
	}
protected:
	void* _bShmData = nullptr;
	std::string _sShmName = "";
	size_t _iMaxShmSize = 0; 
	HANDLE _hMapFile = NULL; 
};


// shared memory 很适合采用singleton模式，可以避免无谓的空间使用
// 但是单例需要用name来控制,而且涉及到垃圾回收和初始化机制
class SharedMemorySingleton : public SharedMemory
{
public:
	static SharedMemorySingleton* Instance(const std::string& name_, const size_t size_)
	{
		if (_singleton.find(name_) == _singleton.end())
		{
			_singleton[name_] = new SharedMemorySingleton(name_, size_);
		}
		else
		{
			// 如果已经存在，但是分配空间不同，请先删除原来的对象
			// example: 
			// SharedMemorySingleton::remove(name_)
			// SharedMemorySingleton::Instance(name_,size_)
			if (size_ != _singleton[name_]->_iMaxShmSize)
				return nullptr; 
		}
		return _singleton[name_];
	}

	static void remove(const std::string& name_)
	{
		if (_singleton.find(name_) != _singleton.end())
		{
			delete _singleton[name_];
			_singleton.erase(name_);
		}
	}

protected:
	SharedMemorySingleton(const std::string& name_, const size_t size_): SharedMemory(name_, size_) {};
	virtual ~SharedMemorySingleton()
	{
	}

private:
	// 注意需要在cpp中初始化static对象
	static std::map<std::string, SharedMemorySingleton*> _singleton; 


	// 垃圾回收
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

#endif