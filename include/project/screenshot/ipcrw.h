#ifndef _IPCRW_H
#define _IPCRW_H
#include "mutex.h"
#include "shared_memory.h"
#include "semaphore.h"

/* brief: Inter process Connection Read Write Class, 
assume one-reader-one-writer(M_1R1W) model or multi-reader-one-writer model(M_MR1W)
*/
class RWBasic
{
public:
	RWBasic(const std::string& name_, const size_t size_) : _sName(name_), _iShmSize(size_)
	{
		_mutex = std::make_shared<Mutex>(name_ + "_Mutex");
		_shmem = std::make_shared<SharedMemory>(name_ + "SharedMemory", size_);
	}
	virtual ~RWBasic()
	{
		_mutex.reset(); 
		_shmem.reset(); 
	}
	virtual bool start()
	{
		if (_mutex->open() && _shmem->open()) {
			return true;
		}
		return false;
	}
	virtual void read_lock() = 0; 
	virtual void write_lock() = 0; 
	virtual void read_unlock() = 0; 
	virtual void write_unlock() = 0; 
	
	virtual BYTE* data()
	{
		return _shmem->data<BYTE>();
	}
protected:
	std::string _sName;
	size_t _iShmSize;
	std::shared_ptr<Mutex> _mutex;
	std::shared_ptr<SharedMemory> _shmem;
};

class OROW : public RWBasic
{
public:
	OROW(const std::string& name_, const size_t size_) : RWBasic(name_, size_) {}
	~OROW() override {}

	void read_lock() override
	{
		_mutex->lock();
	}

	void write_lock() override
	{
		_mutex->lock(); 
	}

	void read_unlock() override
	{
		_mutex->unlock(); 
	}

	void write_unlock() override
	{
		_mutex->unlock(); 
	}
};

class MROW : public RWBasic
{
public:
	MROW(const std::string& name_, const size_t size_) : RWBasic(name_, size_)
	{
		_semaphore = std::make_shared<Semaphore>(name_ + "_Semaphore", 1);
	}
	~MROW() override
	{
		_semaphore.reset(); 
	}

	bool start() override
	{
		if (RWBasic::start() && _semaphore->open())
		{
			return true; 
		}
		return false; 
	}

	void read_lock() override
	{
		Lock lock(_mutex.get());
		int& read_count = reinterpret_cast<int*>(_shmem->data<BYTE>())[0];
		read_count += 1;
		if (read_count == 1)
			_semaphore->wait();
		lock.unlock(); 
	}

	void write_lock() override
	{
		_semaphore->wait(); 
	}

	void read_unlock() override
	{
		Lock lock(_mutex.get());
		int& read_count = reinterpret_cast<int*>(_shmem->data<BYTE>())[0];
		read_count -= 1;
		if (read_count == 0)
			_semaphore->post();
		lock.unlock();
	}

	void write_unlock() override
	{
		_semaphore->post(); 
	}

	BYTE* data() override
	{
		return _shmem->data<BYTE>() + sizeof(int);
	}

private:
	std::shared_ptr<Semaphore> _semaphore; 
};

class IPCRW
{
public:
	IPCRW(const std::string& name_, size_t size_, int rw_type = 1)
	{
		if (rw_type == 0)
		{
			_pImpl = new OROW(name_, size_);
		}
		if (rw_type == 1)
		{
			_pImpl = new MROW(name_, size_);
		}
	}
	~IPCRW()
	{
		delete _pImpl; 
	}
	bool start()
	{
		return _pImpl->start(); 
	}
	void read(BYTE* buffer, size_t size_)
	{
		_pImpl->read_lock(); 
		memcpy(buffer, _pImpl->data(), size_);
		_pImpl->read_unlock(); 
	}
	void write(BYTE* buffer, size_t size_)
	{
		_pImpl->write_lock(); 
		memcpy(_pImpl->data(), buffer, size_);
		_pImpl->write_unlock(); 
	}
	
	template<typename Func>
	void read_from_sm(Func pf)
	{
		_pImpl->read_lock(); 
		pf(_pImpl->data());
		_pImpl->read_unlock(); 
	}

	// 2023/09/09 : to do 如何使用Event来避免忙等？
	// 2023/09/11 : done
	template<typename Func>
	void write_to_sm(Func pf)
	{
		_pImpl->write_lock(); 
		pf(_pImpl->data()); 
		_pImpl->write_unlock(); 
	}
private:
	RWBasic* _pImpl; 
};

#endif