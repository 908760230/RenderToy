#pragma once

#define WIN32_LEAN_AND_MEAN  // just say no to MFC
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <math.h>
#include <string>

class MainWindow
{
public:
	MainWindow(HINSTANCE hinstance);
	MainWindow(const MainWindow &) = delete;

	int show();
	void resize(int w, int h);
	void setWindowTitle(std::string name);
private:
	void messagePump();
	friend LRESULT CALLBACK WindowProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam);
	HINSTANCE hinstance;
	WNDCLASSEX winclass; // this will hold the class we create
	HWND	   hwnd;	 // generic window handle
	MSG		   msg;		 // generic message

	int height=400;
	int width=400;
	std::string windowTitle;
};
