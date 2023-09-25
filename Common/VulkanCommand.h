#pragma once
#include "vulkan/vulkan.h"
#include "VulkanDevice.h"
#include "VulkanBuffer.h"

class VulkanCommand
{
public:
	VulkanCommand(VulkanDevice &vulkanDevice);
	~VulkanCommand();

	void copyBuffer(const VulkanBuffer& srcBuffer, const VulkanBuffer& dstBuffer);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
	void generateMipMaps(VkImage image, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

private:
	void submit();
private:
	VkDevice m_logicalDevice = nullptr;
	VkCommandBuffer m_commandBuffer = nullptr;
	VkCommandPool m_commandPool = nullptr;
	VkQueue m_graphicsQueue = nullptr;
};

