#pragma once

#include <string>
#include "MainWindow.h"
#include "Engine.h"
class Application
{
public:
	Application();
	~Application();

	void init();
	void run();

	virtual void start() {};
	virtual void draw() {};
	virtual std::string getWindowTile();

private:
	MainWindow m_mainWindow;
	Engine	m_engine;
};

