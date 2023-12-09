#ifndef VULKAN_TEXTURE_H
#define VULKAN_TEXTURE_H

#include <ktx.h>
#include <ktxvulkan.h>
#include <string>

#include "VulkanDevice.h"
#include "VulkanImage.h"

class VulkanTexture {
public:
	VulkanDevice* device;
	VulkanImage image;
	VkSampler sampler;

	bool loadKTXFile(std::_Invoker_strategy& filename, ktxTexture** target);
};


class Texture2D : public VulkanTexture {

};
#endif // !TEXTURE_H
