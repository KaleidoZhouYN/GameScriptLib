#include "injector.h"
#include <iostream>


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


/* brief : ע��ĳ��dllpath��Injector��Ӧ��process
* @param  dllpath : ��Ӧ��dll����·��
*/
void Injector::inject(const std::string& dllpath)
{
	proc.Attach(_hProcessId);

	// dllname ����dllpath���ļ���
	
	_dllname = converter.from_bytes(getFilename(dllpath));
	auto _dllptr = proc.modules().GetModule(_dllname);
	bool injected = false; 
	if (_dllptr)
		injected = true; 
	else {
		if (PathExists(dllpath)) {
			auto iret = proc.modules().Inject(converter.from_bytes(dllpath));
			injected = (iret ? true : false);
			if (!injected)
				LOG(ERROR) << "Inject Fail with dllpath : " << dllpath;
			_injected = injected; 
		}
		else {
			LOG(ERROR) << "Path not exist with dllpath : " << dllpath;
		}
	}

	// 2023/08/25 to do : ����inject �� remotefunctioncall
	if (injected) {
		auto _n_dllptr = proc.modules().GetModule(_dllname);
		if (!_n_dllptr) {
			LOG(ERROR) << "Could not found dll module with dllname: " << _dllname;
			return; 
		}
			
		using my_func_t = long(__stdcall*)(DWORD , const size_t);
		auto pSetXHook = blackbone::MakeRemoteFunction<my_func_t>(proc, _dllname, "SetHook");
		if (pSetXHook) {
			const size_t size_ = 2560 * 1600 * 3;
			auto cret = pSetXHook(_hProcessId,size_);
			LOG(INFO) << "Set Hook";
		}
		else
		{
			LOG(INFO) << "Set Hook Fail"; 
		}
	}
	proc.Detach(); 
}

void Injector::release()
{
	// ֻ��ע��ɹ���release��������ע��ʱ�ͷ�֮ǰ��dll
	if (_injected) {
		proc.Attach(_hProcessId);
		using my_func_t = long(__stdcall*)(DWORD);
		auto pUnXHook = blackbone::MakeRemoteFunction<my_func_t>(proc, _dllname, "ReleaseHook");
		if (pUnXHook)
		{
			pUnXHook(_hProcessId);
			LOG(INFO) << "Release hook with processId : " << _hProcessId;
			//MessageBoxA(0, "Release Hook", "OK", MB_ICONEXCLAMATION);
		}
		else {

		}

		// unload inject dll
		auto _dllptr = proc.modules().GetModule(_dllname);
		proc.modules().Unload(_dllptr);
		_injected = false;
		proc.Detach();
		//MessageBoxA(0, "Release dll", "OK", MB_ICONEXCLAMATION);
		LOG(INFO) << "Release dll with processId : " << _hProcessId; 
	}

	
	
}