/**
* @author: KaleidoZ @ 2023/08/06
*/

#include "screenshot.h"

int CaptureWindowScreenshotCPU(HWND hWnd, const char* savePath)
{
	// 获取窗口的DPI
	UINT dpi = GetDpiForWindow(hWnd);
	float scale = static_cast<float>(dpi) / STD_DPI;


	// 获取窗口 Device Context 
	// 设备上下文(DC)是一个GDI（图形设备接口）结构，它定义了一系列的绘图操作，如线条绘制、填充、文本输出
	HDC hwindowDC = GetDC(hWnd);

	// 获取窗口大小
	RECT windowRect; 
	GetWindowRect(hWnd, &windowRect);
	int width = windowRect.right - windowRect.left; 
	int height = windowRect.bottom - windowRect.top; 

	width = static_cast<int>(width * scale);
	height = static_cast<int>(height * scale);

	// 创建兼容的DC和位图
	HDC hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
	HBITMAP hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);

	// 选择新创建的位图对象
	HGDIOBJ oldBitmap = SelectObject(hwindowCompatibleDC, hbwindow);

	// 使用 PrintWindow
	PrintWindow(hWnd, hwindowCompatibleDC, PW_CLIENTONLY);

	BITMAPINFO bi = { 0 };
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = width; 
	bi.bmiHeader.biHeight = -height; 
	bi.bmiHeader.biPlanes = 1; 
	bi.bmiHeader.biBitCount = 32; 
	bi.bmiHeader.biCompression = BI_RGB;

	RGBQUAD* pPixels = new RGBQUAD[width * height];
	if (!GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, pPixels, &bi, DIB_RGB_COLORS)) {
		delete[] pPixels;
		SelectObject(hwindowCompatibleDC,oldBitmap);
		DeleteObject(hbwindow);
		DeleteDC(hwindowCompatibleDC);
		ReleaseDC(hWnd, hwindowDC);
	}

	BITMAPFILEHEADER fileHeader = { 0 };
	fileHeader.bfType = 0x4D42; 
	fileHeader.bfSize = sizeof(fileHeader) + sizeof(bi.bmiHeader) + width * height; 
	fileHeader.bfOffBits = sizeof(fileHeader) + sizeof(bi.bmiHeader);

	FILE* file = fopen(savePath, "wb");
	if (!file) return -1; 

	fwrite(&fileHeader, 1, sizeof(fileHeader), file); 
	fwrite(&bi.bmiHeader, 1, sizeof(bi.bmiHeader), file);
	fwrite(pPixels, 1, width * height * 4, file);
	fclose(file);

	// 清理
	delete[] pPixels;
	SelectObject(hwindowCompatibleDC, oldBitmap);
	DeleteObject(hbwindow);
	DeleteDC(hwindowCompatibleDC);
	ReleaseDC(hWnd, hwindowDC);

	return 0; 
}

int CaptureWindowScreenshotOpenGL(HWND hWnd, const char* savePath)
{
	return 0; 
}

// 将像素数据保存为BMP文件
/*
* @param: data 数据流
* @param: width 宽度
* @param: height 高度
* @param: path 保存的bmp文件路径（也可以是jpg）
*/
void SaveToBMP(const BYTE* data, int width, int height, const char* path)
{
	BITMAPFILEHEADER fileHeader = { 0 };
	BITMAPINFOHEADER infoHeader = { 0 };

	fileHeader.bfType = 0x4D42;  // 'BM' in hexadecimal
	fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + width * height * 3;
	fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	infoHeader.biSize = sizeof(BITMAPINFOHEADER);
	infoHeader.biWidth = width;
	infoHeader.biHeight = height;
	infoHeader.biPlanes = 1;
	infoHeader.biBitCount = 24;  // 24-bit RGB data
	infoHeader.biCompression = BI_RGB;

	FILE* file = fopen(path, "wb");
	if (!file) return;

	fwrite(&fileHeader, sizeof(fileHeader), 1, file);
	fwrite(&infoHeader, sizeof(infoHeader), 1, file);
	fwrite(data, width * height * 3, 1, file);

	fclose(file);
}