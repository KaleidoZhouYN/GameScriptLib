#ifndef _SHARED_MEMORY_H
#define _SHARED_MEMORY_H

#include <string>
#include <map>
#include <memory>
#include <windows.h>


// 通常我们使用共享内存的时候，都只会创建一个真实内存，以及若干个映射的虚拟内存地址
// 所以真实内存是唯一的，但是虚拟内存地址是不一样的，在创建的时候需要注意
// MapViewOfFile 会创建很多隔离地址，需要避免这种情况，采取单例模式
class SharedMemory
{
	static std::map<std::string, int> ref_count;
public:
	SharedMemory()  = default; 
	SharedMemory(const std::string& name_, size_t size_) :_bShmData(nullptr), _sShmName(name_), _iMaxShmSize(size_) { };
	~SharedMemory() {
		close(); 
	}

	SharedMemory(const SharedMemory& rhs) = delete;
	SharedMemory& operator=(const SharedMemory& rhs) = delete;


	bool open();
	void close(); 
	bool resize(size_t size_);
	template<typename T>
	T* data() {
		return static_cast<T*>(_bShmData.get());
	}
private:
	std::shared_ptr<void*> _bShmData = nullptr; // 一个map 同样可以对应多个对象
	std::string _sShmName = "";
	size_t _iMaxShmSize = 0; 

#ifndef HAS_BOOST
	HANDLE _hMapFile; 
#endif
};

// shared memory 很适合采用singleton模式，可以避免无谓的空间使用
class SharedMemorySingleton
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
	SharedMemorySingleton(const std::string& name_, const size_t size_): _bShmData(nullptr), _sShmName(name_), _iMaxShmSize(size_), _hMapFile(NULL) {};
	~SharedMemorySingleton()
	{
		if (_hMapFile)
		{
			UnmapViewOfFile(_hMapFile); // 释放虚拟内存
			CloseHandle(_hMapFile);
		}
	}

private:
	static std::map<std::string, SharedMemorySingleton*> _singleton; 

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
		// 使用 windows API的代码
		// 在一个process中，相同的shmName得到的handle都是相同的
		// 如果sShnName之前就存在，同样会返回handle
		_hMapFile = CreateFileMapping(
			INVALID_HANDLE_VALUE,
			NULL,
			PAGE_READWRITE,
			0,
			_iMaxShmSize,
			_sShmName.c_str());

		if (_hMapFile == NULL) {
			// add log here
			MessageBoxA(0, "Could not create file mapping object", "Fail", MB_ICONEXCLAMATION);
			return FALSE;
		}

		_bShmData = static_cast<void*>(MapViewOfFile(_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, _iMaxShmSize));

		if (_bShmData == nullptr) {
			// add log here
			MessageBoxA(0, "Could not map view of file.", "Fail", MB_ICONEXCLAMATION);
			return FALSE;
		}

		return TRUE; 
	}

	template<typename T>
	T* data() {
		return static_cast<T*>(_bShmData);
	}

private:
	void* _bShmData = nullptr; // 一个map 同样可以对应多个对象
	std::string _sShmName = "";
	size_t _iMaxShmSize = 0;
	HANDLE _hMapFile = NULL;
};

#endif