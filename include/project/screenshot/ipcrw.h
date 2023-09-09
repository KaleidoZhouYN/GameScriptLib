#ifndef _IPCRW_H
#define _IPCRW_H
#include "mutex.h"
#include "shared_memory.h"

/* brief: Inter process Connection Read Write Class */
class IPCRW
{
public:
	IPCRW(const std::string& name_, const size_t size_) : _sName(name_), _nShmSize(size_)
	{
		_mutex = std::make_shared<Mutex>(name_ + "_Mutex");
		_shmem = std::make_shared<SharedMemory>(name_ + "SharedMemory", size_);
	}
	~IPCRW()
	{
		_mutex.reset(); 
		_shmem.reset(); 
	}
	bool start()
	{
		if (_mutex->open() && _shmem->open()) {
			return true; 
		}
		return false; 
	}
	void read(BYTE* buffer, size_t size_)
	{
		// read_count + 1
		Lock lock(_mutex.get());
		int& read_count = reinterpret_cast<int*>(_shmem->data<BYTE>())[0];
		read_count += 1; 
		lock.unlock(); 

		memcpy(buffer, _shmem->data<BYTE>() + sizeof(int), size_);
		
		// read_count - 1
		lock.lock();
		read_count -= 1; 
		lock.unlock(); 
	}
	void write(BYTE* buffer, size_t size_)
	{
		while (true)
		{
			Lock lock(_mutex.get());
			int &read_count = reinterpret_cast<int*>(_shmem->data<BYTE>())[0];
			if (read_count == 0)
			{
				memcpy(_shmem->data<BYTE>() + sizeof(int), buffer, size_);
				lock.unlock(); 
				break;
			}
			lock.unlock(); 
		}
	}
	
	template<typename Func>
	void read_from_sm(Func pf)
	{
		// read_count + 1
		Lock lock(_mutex.get());
		int& read_count = _shmem->data<int>()[0];
		read_count += 1;
		lock.unlock();

		// 考虑异常安全
		try {
			pf(_shmem->data<BYTE>() + sizeof(int));
		}
		catch (std::exception& e)
		{
			lock.lock();
			read_count -= 1;
			lock.unlock();

			throw e; 
		}

		lock.lock(); 
		read_count -= 1;
		lock.unlock();
	}

	// 2023/09/09 : to do 如何使用Event来避免忙等？
	template<typename Func>
	void write_to_sm(Func pf)
	{
		while (true)
		{
			Lock lock(_mutex.get());
			int& read_count = _shmem->data<int>()[0];
			if (read_count == 0)
			{
				pf(_shmem->data<BYTE>() + sizeof(int)); 
				lock.unlock();
				break;
			}
			lock.unlock();
		}
	}
private:
	std::string _sName; 
	std::size_t _nShmSize; 
	std::shared_ptr<Mutex> _mutex; 
	std::shared_ptr<SharedMemory> _shmem; 
};

/* brief: a RAII class for mutex and sharedmemory */
class ScreenShotHook
{
public:
	ScreenShotHook(const std::string& name_, const size_t size_) :_sName(name_)
	{
		//mutex = MutexSingleton::Instance(name_ + "_Mutex");
		//shm = SharedMemorySingleton::Instance(name_ + "SharedMemory", size_);
		mutex = new Mutex(name_ + "_Mutex");
		shm = new SharedMemory(name_ + "_SharedMemory", size_);
	}
	bool start()
	{
		if (!mutex->open()) {
			return false;
		}
			
		if (!shm->open()) {
			return false; 
		}

		return TRUE; 
	}
	~ScreenShotHook()
	{
		//MutexSingleton::remove(_sName);
		//SharedMemorySingleton::remove(_sName);
		delete mutex;
		delete shm;
	}

	//ScreenShotHook(ScreenShotHook& rhs) = delete; 
	template<typename T>
	T* data()
	{
		return shm->data<T>(); 
	}

	Mutex* get_mutex()
	{
		return mutex; 
	}
private:
	const std::string& _sName; 
	Mutex* mutex;
	SharedMemory* shm;
};

#endif