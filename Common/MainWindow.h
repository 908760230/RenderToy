#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN  // just say no to MFC
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <string>
#include <glm/glm.hpp>

class Engine;
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
	MainWindow(Engine &engine);
	MainWindow(const MainWindow&) = delete;
	MainWindow& operator=(const MainWindow&) = delete;

	int show();
	void resize(int w, int h);
	void setWindowTitle(std::string name);
	std::string windowTitle() const { return m_windowTitle; }
	bool processEvent();
	HWND windowHandle() const { return hwnd; }
	int height() const { return m_height; }
	int width() const { return m_width; }
	MouseInfo& mouseInfo() { return m_mouseInfo; }
	bool exited = false;
private:
	static LRESULT  WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	Engine& m_engine;
	WNDCLASSEX winclass; // this will hold the class we create
	HWND	   hwnd;	 // generic window handle
	MSG		   msg;		 // generic message

	int m_height = 600;
	int m_width = 800;
	std::string m_windowTitle;
	MouseInfo m_mouseInfo;
};
