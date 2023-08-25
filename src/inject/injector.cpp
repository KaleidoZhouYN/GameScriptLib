#include "injector.h"
#include <iostream>

bool PathExists(const std::string& s) {
	DWORD dwAttrib = GetFileAttributes(s.c_str());
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

std::string getFilename(const std::string& path) {
	size_t found = path.find_last_of("/\\");
	return path.substr(found + 1);
}

void Injector::inject(const std::string& dllpath)
{
	proc.Attach(_hProcessId);

	// dllname 就是dllpath的文件名
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	_dllname = converter.from_bytes(getFilename(dllpath));
	std::cout << "dllname:" << getFilename(dllpath) << std::endl;
	auto _dllptr = proc.modules().GetModule(_dllname);
	bool injected = false; 
	if (_dllptr)
		injected = true; 
	else {
		if (PathExists(dllpath)) {
			auto iret = proc.modules().Inject(converter.from_bytes(dllpath));
			injected = (iret ? true : false);
			if (!injected)
				std::cout << "注入失败" << std::endl; 
			_injected = injected; 
		}
		else {
			std::cout << "Path not exist" << std::endl;
		}
	}
	if (injected) {
		auto _n_dllptr = proc.modules().GetModule(_dllname);
		if (!_n_dllptr)
			std::cout << "没找到dll模块" << std::endl; 
		using my_func_t = long(__stdcall*)(DWORD , const size_t);
		auto pSetXHook = blackbone::MakeRemoteFunction<my_func_t>(proc, _dllname, "SetHook");
		if (pSetXHook) {
			const size_t size_ = 2560 * 1600 * 3;
			auto cret = pSetXHook(_hProcessId,size_);
			std::cout << "Set Hook" << std::endl; 
		}
		else
		{
			std::cout << "Set Hook Fail" << std::endl; 
		}
	}
	proc.Detach(); 
}

void Injector::release()
{
	if (_injected) {
		proc.Attach(_hProcessId);
		using my_func_t = long(__stdcall*)(DWORD);
		auto pUnXHook = blackbone::MakeRemoteFunction<my_func_t>(proc, _dllname, "ReleaseHook");
		if (pUnXHook)
		{
			pUnXHook(_hProcessId);
			MessageBoxA(0, "Release Hook", "OK", MB_ICONEXCLAMATION);
		}
		else {

		}

		// kill inject dll

		auto _dllptr = proc.modules().GetModule(_dllname);
		proc.modules().Unload(_dllptr);
		_injected = false;
		proc.Detach();
		MessageBoxA(0, "Release dll", "OK", MB_ICONEXCLAMATION);
	}

	
	
}