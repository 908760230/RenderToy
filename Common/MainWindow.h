#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN  // just say no to MFC
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <string>
#include <glm/glm.hpp>


struct MouseInfo
{
	glm::vec2 m_mousePos{};
	bool leftDown = false;
	bool rightDown = false;
	bool wheelDown = false;
};


class MainWindow
{
public:
	MainWindow();
	MainWindow(const MainWindow&) = delete;
	MainWindow& operator=(const MainWindow&) = delete;

	int show();
	void resize(int w, int h);
	void setWindowTitle(std::string name);
	bool processEvent();
	HWND windowHandle() const { return hwnd; }
	int height() const { return m_height; }
	int width() const { return m_width; }
	MouseInfo& mouseInfo() { return m_mouseInfo; }
	bool exited = false;
private:
	WNDCLASSEX winclass; // this will hold the class we create
	HWND	   hwnd;	 // generic window handle
	MSG		   msg;		 // generic message

	int m_height = 600;
	int m_width = 800;
	std::string windowTitle;
	MouseInfo m_mouseInfo;
};
