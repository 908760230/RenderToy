#pragma once
#include <vulkan/vulkan.h>
#include "VulkanDevice.h"

class VulkanBuffer
{
public:
	VulkanBuffer(VulkanDevice &m_vulkanDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
	~VulkanBuffer();
	void mapMemory();
	VkBuffer buffer() const { return m_buffer; }
	void copyBuffer(const VulkanBuffer& other);
	VkDeviceSize size() const { return m_bufferSize; }
	void* data() { return m_data; };
	void update(void* source);
	void flushMemory();

private:

	void clear();
	void unMap();
private:
	VulkanDevice &m_vulkanDevice;
	VkBuffer m_buffer = nullptr;
	VkDeviceMemory m_bufferMemory = nullptr;
	VkDeviceSize m_bufferSize = 0;
	VkDeviceSize m_memorySize;
	VkBufferUsageFlags m_usage{};
	bool m_mapped = false;
	void* m_data = nullptr;
};

