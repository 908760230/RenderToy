#include "TextureApplication.h"

TextureApplication::~TextureApplication()
{
}

void TextureApplication::prepare()
{
	createSyncObjects();
	buildCommandBuffers();
}
