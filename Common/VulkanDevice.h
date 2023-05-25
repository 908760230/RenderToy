#pragma once

#include <vulkan/vulkan.h>
#include <optional>
#include <vector>

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

class VulkanDevice
{
private:

public:
	VulkanDevice();
	~VulkanDevice();
	VulkanDevice(const VulkanDevice& other);
	void createDevice(VkPhysicalDevice physicalDevice,VkSurfaceKHR surface, bool validation = false);
	VkDevice logicalDevice() const { return m_logicalDevice; }
	VkCommandPool commandPool() const{ return m_commandPool; }
	VkPhysicalDevice physicalDevice() const { return m_physicalDevice; }
	VkQueue graphicQueue() const { return m_graphicsQueue; }
	VkQueue presentQueue() const { return m_presentQueue; }
	VkSurfaceKHR surface() const { return m_surface; }
	std::vector<uint32_t> queueFamilyIndices() const { return m_queueFamilyIndices; }
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
	VkSampleCountFlagBits maxUsableSampleCount() const { return m_msaa; }
	void wait();
	VkFormat findDepthFormat();
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	VkPhysicalDeviceProperties properties;
private:
	VkSampleCountFlagBits getMaxUsableSampleCount();
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

private:
	VkSurfaceKHR m_surface = nullptr;
	VkQueue m_graphicsQueue = nullptr;
	VkQueue m_presentQueue = nullptr;
	VkCommandPool m_commandPool = nullptr;
	VkPhysicalDevice m_physicalDevice = nullptr;
	VkDevice m_logicalDevice = nullptr;
	std::vector<uint32_t> m_queueFamilyIndices;
	VkSampleCountFlagBits m_msaa = VK_SAMPLE_COUNT_1_BIT;
};

