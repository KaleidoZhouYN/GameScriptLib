#include "injector.h"
#include <iostream>
std::unordered_map<std::string, int> Injector::ref_count = {};

// global variable
std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

// global functions
bool PathExists(const std::string& s) {
	DWORD dwAttrib = GetFileAttributes(s.c_str());
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

std::string getFilename(const std::string& path) {
	size_t found = path.find_last_of("/\\");
	return path.substr(found + 1);
}


/* brief : 注入某个dllpath到Injector对应的process
* @param  dllpath : 对应的dll绝对路径
*/
void Injector::inject(const std::string& dllpath)
{
	if (_injected)
		return;
	proc.Attach(_hProcessId);

	// dllname 就是dllpath的文件名

	_dllname = getFilename(dllpath);
	_wdllname = converter.from_bytes(_dllname);
	auto _dllptr = proc.modules().GetModule(_wdllname);
	if (_dllptr)
		_injected = true;
	else {
		if (PathExists(dllpath)) {
			auto iret = proc.modules().Inject(converter.from_bytes(dllpath));
			_injected = (iret ? true : false);
			if (!_injected)
				LOG(ERROR) << "Inject Fail with dllpath : " << dllpath;
		}
		else {
			LOG(ERROR) << "Path not exist with dllpath : " << dllpath;
		}
	}

	if (_injected) {
		std::stringstream ss;
		ss << _hProcessId << "_" << _dllname;
		if (ref_count.find(ss.str()) == ref_count.end())
		{
			ref_count[ss.str()] = 0;
		}
		ref_count[ss.str()] += 1;
	}

}

// 2023/08/25 to do : 分离inject 和 remotefunctioncall
// done 2023/08/30
// to do : 2023/09/01 : 给 set_hook 增加参数, 统一接口
void Injector::set_hook(const std::string& func_name, LPARAM lParam)
{
	if (_injected) {
		auto _n_dllptr = proc.modules().GetModule(_wdllname);
		if (!_n_dllptr) {
			LOG(ERROR) << "Could not found dll module with dllname: " << _dllname;
			return; 
		}
			
		using my_func_t = long(__stdcall*)(LPARAM);
		auto pSetXHook = blackbone::MakeRemoteFunction<my_func_t>(proc, _wdllname, func_name.c_str());
		if (pSetXHook) {
			auto cret = pSetXHook(lParam);
			LOG(INFO) << "Set Hook";
		}
		else
		{
			LOG(INFO) << "Set Hook Fail"; 
		}
	}
}

void Injector::release_hook(const std::string& func_name, LPARAM lParam)
{
	if (_injected) {
		using my_func_t = long(__stdcall*)(LPARAM);
		auto pUnXHook = blackbone::MakeRemoteFunction<my_func_t>(proc, _wdllname, func_name.c_str());
		if (pUnXHook)
		{
			pUnXHook(lParam);
			LOG(INFO) << "Release hook with processId : " << _hProcessId;
			//MessageBoxA(0, "Release Hook", "OK", MB_ICONEXCLAMATION);
		}
		else {

		}
	}
}

void Injector::set_capture_hook(size_t MaxShmSize)
{
	if (_injected) {
		auto _n_dllptr = proc.modules().GetModule(_wdllname);
		if (!_n_dllptr) {
			LOG(ERROR) << "Could not found dll module with dllname: " << _dllname;
			return;
		}

		using my_func_t = long(__stdcall*)(DWORD , size_t);
		auto pSetXHook = blackbone::MakeRemoteFunction<my_func_t>(proc, _wdllname, "SetHook");
		if (pSetXHook) {
			auto cret = pSetXHook(_hProcessId, MaxShmSize);
			LOG(INFO) << "Set Capture Hook";
		}
		else
		{
			LOG(INFO) << "Set Capture Hook Fail";
		}
	}
}

void Injector::release_capture_hook()
{
	if (_injected) {
		using my_func_t = long(__stdcall*)();
		auto pUnXHook = blackbone::MakeRemoteFunction<my_func_t>(proc, _wdllname, "ReleaseHook");
		if (pUnXHook)
		{
			pUnXHook();
			LOG(INFO) << "Release Capture hook"; 
		}
		else {

		}
	}
}

void Injector::set_message_hook(DWORD pid)
{
	if (_injected) {
		auto _n_dllptr = proc.modules().GetModule(_wdllname);
		if (!_n_dllptr) {
			LOG(ERROR) << "Could not found dll module with dllname: " << _dllname;
			return;
		}

		using my_func_t = long(__stdcall*)(DWORD);
		auto pSetXHook = blackbone::MakeRemoteFunction<my_func_t>(proc, _wdllname, "SetHook");
		if (pSetXHook) {
			auto cret = pSetXHook(pid);
			LOG(INFO) << "Set Message Hook";
		}
		else
		{
			LOG(INFO) << "Set Message Hook Fail";
		}
	}
}

void Injector::release_message_hook()
{
	if (_injected) {
		using my_func_t = long(__stdcall*)();
		auto pUnXHook = blackbone::MakeRemoteFunction<my_func_t>(proc, _wdllname, "ReleaseHook");
		if (pUnXHook)
		{
			pUnXHook();
			LOG(INFO) << "Release message hook";
		}
		else {

		}
	}
}

void Injector::release()
{
	// 只有注入成功才release，避免多次注入时释放之前的dll
	if (_injected)
	{
		// unload inject dll
		auto _dllptr = proc.modules().GetModule(_wdllname);
		proc.modules().Unload(_dllptr);
		_injected = false;
		proc.Detach();
		//MessageBoxA(0, "Release dll", "OK", MB_ICONEXCLAMATION);
		LOG(INFO) << "Release dll with processId : " << _hProcessId; 

		std::stringstream ss;
		ss << _hProcessId << "_" << _dllname;
		ref_count[ss.str()] -= 1; 
		if (ref_count[ss.str()] < 1)
		{
			ref_count.erase(ss.str());
		}
	}
}