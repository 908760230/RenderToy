#pragma once
#include "Engine.h"
#include "TextOverlay.h"

#include <memory>

class TextApplication : public Engine
{
public:
	TextApplication();
	~TextApplication();

protected:
	void prepare() override;

private:
	std::shared_ptr<TextOverlay> m_textOverlay;
};

