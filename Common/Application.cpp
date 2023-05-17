#include "Application.h"

Application::Application()
{
	m_mainWindow.setWindowTitle(getWindowTile());
}

Application::~Application()
{
}

void Application::init()
{
	m_mainWindow.show();
	m_engine.setMainWindow(&m_mainWindow);
	m_engine.init();
}

void Application::run()
{
	m_engine.run();
}

std::string Application::getWindowTile()
{
	return "Render Toy";
}
