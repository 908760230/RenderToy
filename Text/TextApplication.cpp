#include "TextApplication.h"

TextApplication::TextApplication()
{
	m_mainWindow.setWindowTitle("sample_text");
}

TextApplication::~TextApplication()
{
}

void TextApplication::prepare()
{
	createSyncObjects();
	m_textOverlay = std::make_shared<TextOverlay>(m_vulkanDevice, m_swapchain);
}
