#pragma once
#include <vulkan/vulkan.h>
#include "VulkanDevice.h"

struct VulkanImageProperties
{
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t mipLevels = 0; 
	VkFormat format = VK_FORMAT_UNDEFINED;
	VkImageAspectFlags aspectFlags = VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
};

class VulkanImage
{
public:
	VulkanImage() {};
	VulkanImage(VulkanDevice* device) :m_vulkanDevice(device) {};
	~VulkanImage();
	void setVulkanDevice(VulkanDevice* device) { m_vulkanDevice = device; }
	void createImage(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage);
	VulkanImageProperties getProperties() const { return m_properties; };
	VkImageView imageView() const { return m_imageView; }
	VkImage image() const { return m_image; }
	void generateMipMaps(VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

private:
	void createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
private:
	VulkanDevice* m_vulkanDevice = nullptr;
	VkImage m_image = nullptr;
	VkImageView m_imageView = nullptr;
	VkDeviceMemory m_imageMemory = nullptr;
	VulkanImageProperties m_properties;
};

