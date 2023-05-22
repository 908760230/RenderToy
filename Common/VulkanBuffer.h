#pragma once
#include <vulkan/vulkan.h>
#include "VulkanDevice.h"

class VulkanBuffer
{
public:
	VulkanBuffer(VulkanDevice* m_vulkanDevice = nullptr);
	~VulkanBuffer();
	void setVulkanDevice(VulkanDevice* device) { m_vulkanDevice = device; }
	void createBuffer(void* data, size_t size, VkBufferUsageFlags usage = 0);
	void createBufferWithoutCopy(void* data, size_t size);
	void createUniformBuffer(size_t size);
	void mapMemory(void *source, bool unMap = true);
	VkBuffer buffer() const { return m_buffer; }
	void copyBuffer(const VulkanBuffer& other);
	VkDeviceSize size() const { return m_bufferSize; }
	void* data() { return m_data; };
private:
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

	void clear();
private:
	VulkanDevice* m_vulkanDevice = nullptr;
	VkBuffer m_buffer = nullptr;
	VkDeviceMemory m_bufferMemory = nullptr;
	VkDeviceSize m_bufferSize = 0;
	void* m_data;
};

