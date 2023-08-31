class Controller
{
public:
	Controller(HWND hwnd) : _hwnd(hwnd) {}
	~Controller()
	{

	}


	POINT get_window_HW()
	{
		RECT rect;
		GetClientRect(_hwnd, &rect);
		_width = rect.right - rect.left;
		_height = rect.bottom - rect.top;
		return { _width, _height };
	}

	// 模拟鼠标移动
	void mouse_move(const int x, const int y, int status = 0, const int step = 1, const float time=0.0)
	{
		PostMessage(_hwnd, WM_MOUSEMOVE, status, MAKELPARAM(x, y));
	}

	// 模拟鼠标左键点击
	void mouse_lclick(const int x, const int y, float time=0.1)
	{
		PostMessage(_hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x, y));
		Sleep(time * 1000);
		PostMessage(_hwnd, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(x, y));
	}

	// 模拟鼠标右键点击
	void mouse_rclick(const int x, const int y)
	{
		PostMessage(_hwnd, WM_RBUTTONDOWN, MK_RBUTTON, MAKELPARAM(x, y));
		PostMessage(_hwnd, WM_RBUTTONUP, MK_RBUTTON, MAKELPARAM(x, y));
	}

	// 模拟鼠标左键拖动选取
	void mouse_ldrag(const int x1, const int y1, const int x2, const int y2, const int step = 1, const float time = 0.1)
	{
		PostMessage(_hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x1, y1));
		float stepX = (x2 - x1) / static_cast<float>(step);
		float stepY = (y2 - y1) / static_cast<float>(step);
		for (int i = 0; i < step; ++i) {
			PostMessage(_hwnd, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(x1 + stepX * i, y1 + stepY * i));
			Sleep(time * 1000);
		}
		//PostMessage(_hwnd, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(x2, y2));
		PostMessage(_hwnd, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(x2, y2));
	}

	// 模拟鼠标右键拖动选取
	void mouse_rdrag(const int x1, const int y1, const int x2, const int y2, const int step=1, const float time=0.1)
	{
		PostMessage(_hwnd, WM_RBUTTONDOWN, MK_RBUTTON, MAKELPARAM(x1, y1));
		float stepX = (x2-x1)/static_cast<float>(step);
		float stepY = (y2 - y1) / static_cast<float>(step);
		for (int i = 0; i < step; ++i) {
			PostMessage(_hwnd, WM_MOUSEMOVE, MK_RBUTTON, MAKELPARAM(x1 + stepX * i, y1 + stepY * i));
			Sleep(time * 1000);
		}
		//PostMessage(_hwnd, WM_MOUSEMOVE, MK_RBUTTON, MAKELPARAM(x2, y2));
		PostMessage(_hwnd, WM_RBUTTONUP, MK_RBUTTON, MAKELPARAM(x2, y2));
	}

	// 模拟鼠标左键按下
	void mouse_ldown(const int x, const int y)
	{
		PostMessage(_hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x, y));
	}

	// 模拟鼠标右键按下
	void mouse_rdown(const int x, const int y)
	{
		PostMessage(_hwnd, WM_RBUTTONDOWN, MK_RBUTTON, MAKELPARAM(x, y));
	}


private:
	HWND _hwnd;
	int _width;
	int _height;
};