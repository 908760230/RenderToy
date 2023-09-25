#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "VulkanDevice.h"
#include "VulkanImage.h"
struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};
class VulkanSwapchain
{
public:
	VulkanSwapchain(VulkanDevice* device);
	~VulkanSwapchain();
	void createSwapChain(VulkanDevice* device);
	VkExtent2D extend2D() { return m_swapchainExtent; }
	void clear();
	void recreate();
	void beginFrame(VkCommandBuffer commandBuffer, VkSemaphore semaphore);
	VkResult endFrame( const VkSemaphore* signalSemaphores);
	SwapChainSupportDetails querySwapChainSupport(VulkanDevice* device);
	VkFormat GetSwapchainImageFormat() const {
		return m_swapchainImageFormat;
	}

	const std::vector<VkImageView> &GetImageViews() const {
		return m_swapchainImageViews;
	}
	const VulkanImage* GetDepthImage() const {
		return m_depthImage;
	}

	const VulkanImage* GetSampleImage() const {
		return m_sampleImage;
	}
private:

	void createSampleImage();
	void createDepthImage();
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
private:
	VulkanImage *m_depthImage = nullptr;
	VulkanImage *m_sampleImage = nullptr;
	VulkanDevice* m_vulkanDevice = nullptr;
	VkSwapchainKHR m_swapchain = nullptr;
	VkExtent2D m_swapchainExtent{};
	VkFormat m_swapchainImageFormat = VK_FORMAT_UNDEFINED;

	std::vector<VkImage> m_swapchainImages;
	std::vector<VkImageView> m_swapchainImageViews;

	VkQueue m_presentQueue = nullptr;
	uint32_t m_imageIndex = 0;
};

