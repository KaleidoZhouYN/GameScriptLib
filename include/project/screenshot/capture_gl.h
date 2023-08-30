// define a class for opengl_render application capter
#include "injector.h"
#include "hook.h"
#include "frame_info.h"

// capture 最好是singleton,主线程控制开关
class OpenglCapture
{
public:
	OpenglCapture(HWND hwnd, const std::string dllpath, size_t MaxShmSize) : _hwnd(hwnd), _dllpath(dllpath)
	{
		DWORD hProcessId; 
		GetWindowThreadProcessId(hwnd, &hProcessId);
		_injector = std::make_shared<Injector>(hProcessId);
		std::stringstream ss; 
		ss << "Capture_" << hProcessId;
		_name = ss.str(); 
		_sh = std::make_shared<ScreenShotHook>(_name, MaxShmSize); 
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
		_injector->set_hook(); 
		_sh->start();
		lock.unlock(); 
	}

	void end()
	{
		std::unique_lock<std::mutex> lock(_thread_mtx);
		_injector->release_hook(); 
		_injector->release();  
		lock.unlock(); 
	}

	void capture_frame(FrameInfo*);
private:
	std::string _name;
	HWND _hwnd; // 目标进程的hwnd
	std::shared_ptr<Injector> _injector; 
	std::string _dllpath; 
	std::mutex _thread_mtx; // 多线程的锁
	std::shared_ptr<ScreenShotHook> _sh;
};