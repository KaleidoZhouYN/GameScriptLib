#include "capture_gl.h"


// ÿ���̶߳�Ҫ�½�һ��framInfo
void OpenglCapture::capture_frame(FrameInfo* frame_info)
{
	// read only, not need to consider multi-thread lock

	Lock lock(_sh->get_mutex());
	frame_info->read(_sh->data<BYTE>()); 
	lock.unlock(); 
}