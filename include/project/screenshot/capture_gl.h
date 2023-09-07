// define a class for opengl_render application capter
#include "injector.h"
#include "ipcrw.h"
#include "frame_info.h"

// capture �����singleton,���߳̿��ƿ���
class OpenglCapture
{
public:
	OpenglCapture(HWND hwnd, const std::string dllpath, size_t MaxShmSize) : _hwnd(hwnd), _dllpath(dllpath), _MaxShmSize(MaxShmSize)
	{
		GetWindowThreadProcessId(hwnd, &_hProcessId);
		_injector = std::make_shared<Injector>(hwnd);
		std::stringstream ss; 
		ss << "Capture_" << _hProcessId;
		_name = ss.str(); 
		//_sh = std::make_shared<ScreenShotHook>(_name, MaxShmSize); 
		_ipcrw = std::make_shared<IPCRW>(_name, _MaxShmSize);
	};
	~OpenglCapture() 
	{
		end(); 
	};

	std::string get_name()
	{
		return _name;
	}

	void start()
	{
		std::unique_lock<std::mutex> lock(_thread_mtx);
		_injector->inject(_dllpath);
		
		_injector->set_capture_hook(_MaxShmSize);
		//_sh->start();
		_ipcrw->start(); 
		lock.unlock(); 
	}

	void end()
	{
		std::unique_lock<std::mutex> lock(_thread_mtx);
		_injector->release_capture_hook();
		_injector->release();  
		lock.unlock(); 
	}

	void capture_frame(FrameInfo*);
private:
	std::string _name;
	HWND _hwnd; // Ŀ����̵�hwnd
	std::shared_ptr<Injector> _injector; 
	std::string _dllpath; 
	std::mutex _thread_mtx; // ���̵߳���
	//std::shared_ptr<ScreenShotHook> _sh;
	std::shared_ptr<IPCRW> _ipcrw; 
	DWORD _hProcessId;
	size_t _MaxShmSize;
};