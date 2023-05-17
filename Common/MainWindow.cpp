#include "MainWindow.h"
#include <iostream>

const char* WINDOW_CLASS_NAME = "WINCLASS1";

LRESULT  WindowProc(HWND hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam) { 
	// this is the main message handler of the system
	PAINTSTRUCT		ps;		// used in WM_PAINT
	HDC				hdc;	// handle to a device context

	// what is the message 
	switch (msg)
	{
	case WM_CREATE:
	{
		// 窗体居中显示
		int scrWidth, scrHeight;
		RECT rect;
		//获得屏幕尺寸
		scrWidth = GetSystemMetrics(SM_CXSCREEN);
		scrHeight = GetSystemMetrics(SM_CYSCREEN);
		//取得窗口尺寸
		GetWindowRect(hwnd, &rect);
		//重新设置rect里的值
		rect.left = (scrWidth - rect.right) / 2;
		rect.top = (scrHeight - rect.bottom) / 2;
		//移动窗口到指定的位置
		SetWindowPos(hwnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, SWP_SHOWWINDOW);
	} break;

	case WM_PAINT:
	{
		// simply validate the window
		hdc = BeginPaint(hwnd, &ps);
		// you would do all your painting here
		EndPaint(hwnd, &ps);

		// return success
		return(0);
	} break;

	case WM_DESTROY:
	{
		// kill the application, this sends a WM_QUIT message 
		PostQuitMessage(0);
		// return success
		return(0);
	} break;
	case WM_SIZE: {
		RECT rect;
		GetWindowRect(hwnd, &rect);
		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;
		break;
	}
	default:break;

	} // end switch

	return (DefWindowProc(hwnd, msg, wparam, lparam));
}

MainWindow::MainWindow()
{
	// first fill in the window class stucture
	winclass.cbSize = sizeof(WNDCLASSEX);
	winclass.style = CS_DBLCLKS | CS_OWNDC |
		CS_HREDRAW | CS_VREDRAW;
	winclass.lpfnWndProc = WindowProc;
	winclass.cbClsExtra = 0;
	winclass.cbWndExtra = 0;
	winclass.hInstance = GetModuleHandle(nullptr);
	winclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	winclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winclass.lpszMenuName = NULL;
	winclass.lpszClassName = "WINCLASS1";
	winclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
}

int MainWindow::show()
{
	if (!RegisterClassEx(&winclass)) return 0;
	hwnd = CreateWindowEx(NULL, // extended style
		"WINCLASS1",   // class
		windowTitle.c_str(), // title
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		0, 0,	    // initial x,y
		m_width, m_height,  // initial width, height
		NULL,	    // handle to parent 
		NULL,	    // handle to menu
		nullptr,// instance of this application
		NULL);

	if (!hwnd) return 0;
	return 1;
}

void MainWindow::resize(int w, int h)
{
	m_width = w;
	m_height = h;
}

void MainWindow::setWindowTitle(std::string name)
{
	windowTitle = name;
}

bool MainWindow::processEvent()
{
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return true;
}
