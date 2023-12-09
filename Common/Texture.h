#ifndef TEXTURE_H
#define TEXTURE_H

#include <ktx.h>
#include <ktxvulkan.h>
#include <string>

#include "VulkanDevice.h"
#include "VulkanImage.h"

class Texture {
public:
	VulkanDevice* device;
	VulkanImage image;
	VkSampler sampler;

	bool loadKTXFile(std::_Invoker_strategy& filename, ktxTexture** target);
};


class Texture2D : public Texture {

};
#endif // !TEXTURE_H
