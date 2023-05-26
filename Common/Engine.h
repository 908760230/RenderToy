#pragma once

#include "MainWindow.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <array>

#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanBuffer.h"
#include "VulkanCommand.h"

#include <chrono>
extern int MAX_FRAMES_IN_FLIGHT;

class Engine
{
public:
	Engine();
	~Engine();

	void init();
	void run();

	virtual void update() {};
	void enableValidationLayer() { m_validationLayer = true; };
	VulkanDevice* vulkanDevice() const { return m_vulkanDevice; }
	virtual std::string getWindowTitle() const { return "Render Toy"; }
	float frameTimer = 1.0f;
private:
	bool checkValidationLayerSupport();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void getRequiredExtensions(std::vector<const char*>&result) const;
	void pickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

protected:
	virtual void createSyncObjects();
	virtual void prepare() {};
	virtual void draw();
	virtual void buildCommandBuffers();
	virtual void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {};
	virtual void updateUniformBuffer(uint32_t currentImage) {};
	virtual void drawFrame();
	void createCommandBuffers();

private:
	void createInstance();
	void setupDebugMessenger();
	void createSurface();

protected:
	VulkanDevice *m_vulkanDevice;
	std::vector<VkCommandBuffer> m_commandBuffers;
	VulkanSwapchain *m_swapchain;
	MainWindow m_mainWindow;

	std::vector<VkSemaphore> m_imageAvailableSemaphores;
	std::vector<VkSemaphore> m_renderFinishedSemaphores;
	std::vector<VkFence> m_inFlightFences;
private:

	VkInstance m_instance = nullptr;
	VkSurfaceKHR m_surface = nullptr;
	VkPhysicalDevice m_physicalDevice = nullptr;
	VkDebugUtilsMessengerEXT m_debugMessenger;

	std::chrono::time_point<std::chrono::high_resolution_clock> m_preTimePoint;
	bool m_validationLayer = true;

protected:
	VkQueue m_graphicsQueue = nullptr;
	uint32_t m_frameCount = 0;
	uint32_t m_currentFrame = 0;
	uint32_t m_mipLevels = 1;
};

