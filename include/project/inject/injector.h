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


// 2023/08/24
// to do : decouple interface and implement

class Injector
{
private: 
	static std::unordered_map<std::string, int> ref_count;
public:
	Injector() = default; 
	Injector(HWND hwnd) {
		GetWindowThreadProcessId(hwnd, &_hProcessId);
	}
	Injector(DWORD hProcessId) : _hProcessId(hProcessId) {}
	~Injector()
	{
		release_hook();
		release();
	}
	void set_process(DWORD hProcessId) {
		_hProcessId = hProcessId;
	};
	void inject(const std::string&);
	void release(); 

	void set_hook();
	void release_hook(); 
private:
	blackbone::Process proc;
	std::string _dllname;
	std::wstring _wdllname;
	bool _injected = false; 
	DWORD _hProcessId; 


};
#endif _INJECTOR_H