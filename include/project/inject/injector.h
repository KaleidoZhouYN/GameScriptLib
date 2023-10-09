#ifndef _INJECTOR_H
#define _INJECTOR_H

#include <string>
#include <Windows.h>
#include <locale>
#include <codecvt>
#include "BlackBone/Process/Process.h"
#include "BlackBone/Process/RPC/RemoteFunction.hpp"
#include<unordered_map>

#include "easylogging++.h"
#define ELPP_THREAD_SAFE


class Injector
{
private: 
	static std::unordered_map<std::string, int> ref_count;
public:
	Injector() = default; 
	Injector(HWND hwnd):_hwnd(hwnd) {
		GetWindowThreadProcessId(hwnd, &_hProcessId);
	}
	~Injector()
	{
		release();
	}

	void inject(const std::string&);
	void release(); 

	void set_hook(const std::string&, LPARAM);
	void release_hook(const std::string&, LPARAM); 

	void set_capture_hook(size_t);
	void release_capture_hook(); 

	void set_message_hook(DWORD); 
	void release_message_hook(); 
private:
	blackbone::Process proc;
	std::string _dllname;
	std::wstring _wdllname;
	bool _injected = false; 
	DWORD _hProcessId; 
	HWND _hwnd; 


};
#endif _INJECTOR_H