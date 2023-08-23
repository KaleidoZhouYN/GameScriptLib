#ifndef _SHARED_MEMORY_H
#define _SHARED_MEMORY_H

#include <string>
#include <map>
#include <memory>
#include <windows.h>


// ͨ������ʹ�ù����ڴ��ʱ�򣬶�ֻ�ᴴ��һ����ʵ�ڴ棬�Լ����ɸ�ӳ��������ڴ��ַ
// ������ʵ�ڴ���Ψһ�ģ����������ڴ��ַ�ǲ�һ���ģ��ڴ�����ʱ����Ҫע��
// MapViewOfFile �ᴴ���ܶ�����ַ����Ҫ���������������ȡ����ģʽ
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
	std::shared_ptr<void*> _bShmData = nullptr; // һ��map ͬ�����Զ�Ӧ�������
	std::string _sShmName = "";
	size_t _iMaxShmSize = 0; 

#ifndef HAS_BOOST
	HANDLE _hMapFile; 
#endif
};

// shared memory ���ʺϲ���singletonģʽ�����Ա�����ν�Ŀռ�ʹ��
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
			// ����Ѿ����ڣ����Ƿ���ռ䲻ͬ������ɾ��ԭ���Ķ���
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
			UnmapViewOfFile(_hMapFile); // �ͷ������ڴ�
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
		// ʹ�� windows API�Ĵ���
		// ��һ��process�У���ͬ��shmName�õ���handle������ͬ��
		// ���sShnName֮ǰ�ʹ��ڣ�ͬ���᷵��handle
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
	void* _bShmData = nullptr; // һ��map ͬ�����Զ�Ӧ�������
	std::string _sShmName = "";
	size_t _iMaxShmSize = 0;
	HANDLE _hMapFile = NULL;
};

#endif