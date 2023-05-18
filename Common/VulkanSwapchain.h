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
	VulkanSwapchain();
	~VulkanSwapchain();
	void init(VulkanDevice* device);
	void createSwapChain(VulkanDevice* device);
	VkExtent2D extend2D() { return m_swapchainExtent; }
	void clear();
	void recreate();
	uint32_t beginFrame(VkCommandBuffer commandBuffer, VkSemaphore semaphore);
	void endFrame(uint32_t imageIndex, const VkSemaphore* signalSemaphores);
	SwapChainSupportDetails querySwapChainSupport(VulkanDevice* device);
	VkRenderPass renderPass() const { return m_renderPass; }
	VkFramebuffer frameBuffer(uint32_t index) { return m_swapchainFramebuffers[index]; }
private:
	void createRenderPass();
	void createFrameBuffers();
	void createSampleImage();
	void createDepthImage();
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
private:
	VulkanImage m_depthImage;
	VulkanImage m_sampleImage;
	VulkanDevice* m_vulkanDevice = nullptr;
	VkSwapchainKHR m_swapchain = nullptr;
	VkExtent2D m_swapchainExtent{};
	VkFormat m_swapchainImageFormat = VK_FORMAT_UNDEFINED;

	std::vector<VulkanImage> m_swapchainImages;
	std::vector<VkFramebuffer> m_swapchainFramebuffers;

	VkRenderPass m_renderPass = nullptr;
	VkQueue m_presentQueue = nullptr;
};

