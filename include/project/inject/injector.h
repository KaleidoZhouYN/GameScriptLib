#ifndef _INJECTOR_H
#define _INJECTOR_H

#include <string>
#include <Windows.h>
#include <locale>
#include <codecvt>
#include "BlackBone/Process/Process.h"
#include "BlackBone/Process/RPC/RemoteFunction.hpp"

#include "easylogging++.h"
#define ELPP_THREAD_SAFE


// 2023/08/24
// to do : decouple interface and implement

class Injector
{
public:
	Injector(DWORD hProcessId) : _hProcessId(hProcessId) {}
	~Injector()
	{
		release();
	}
	void inject(const std::string&);
	void release(); 
private:
	blackbone::Process proc;
	std::wstring _dllname;
	bool _injected = false; 
	DWORD _hProcessId; 


};
#endif _INJECTOR_H