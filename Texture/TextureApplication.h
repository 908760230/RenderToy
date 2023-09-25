#pragma once

#include "Engine.h"

class TextureApplication : public Engine {
public:
	TextureApplication() = default;
	~TextureApplication();

	void prepare() override;

};