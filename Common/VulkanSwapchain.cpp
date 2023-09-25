#include "VulkanSwapchain.h"
#include "Engine.h"
#include <stdexcept>

VulkanSwapchain::VulkanSwapchain(VulkanDevice* device)
{
    createSwapChain(device);
}

VulkanSwapchain::~VulkanSwapchain()
{
    clear();
}

void VulkanSwapchain::createSwapChain(VulkanDevice* device)
{
    m_vulkanDevice = device;
    m_presentQueue = device->presentQueue();
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    m_swapchainImageFormat = surfaceFormat.format;
    m_swapchainExtent = extent;

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    MAX_FRAMES_IN_FLIGHT = imageCount;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = device->surface();

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    std::vector<uint32_t> indices = device->queueFamilyIndices();
    if (indices.size() >=2 && indices[0] != indices[1]) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = indices.size();
        createInfo.pQueueFamilyIndices = indices.data();
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    auto result = vkCreateSwapchainKHR(device->logicalDevice(), &createInfo, nullptr, &m_swapchain);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(device->logicalDevice(), m_swapchain, &imageCount, nullptr);
    m_swapchainImages.resize(imageCount);
    m_swapchainImageViews.resize(imageCount);
    vkGetSwapchainImagesKHR(device->logicalDevice(), m_swapchain, &imageCount, m_swapchainImages.data());

    for (size_t i = 0; i < m_swapchainImages.size(); i++) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_swapchainImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_swapchainImageFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device->logicalDevice(), &viewInfo, nullptr, &m_swapchainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image view!");
        }
    }
    createSampleImage();
    createDepthImage();
}

void VulkanSwapchain::clear()
{
    if (m_depthImage) {
        delete m_depthImage;
        m_depthImage = nullptr;
    }
    if (m_sampleImage) {
        delete m_sampleImage;
        m_sampleImage = nullptr;
    }
    for (auto imageView : m_swapchainImageViews) vkDestroyImageView(m_vulkanDevice->logicalDevice(), imageView, nullptr);
    vkDestroySwapchainKHR(m_vulkanDevice->logicalDevice(), m_swapchain, nullptr);
}

void VulkanSwapchain::recreate()
{
    vkDeviceWaitIdle(m_vulkanDevice->logicalDevice());
    clear();
    createSwapChain(m_vulkanDevice);
}

void VulkanSwapchain::beginFrame(VkCommandBuffer commandBuffer , VkSemaphore semaphore)
{
    VkResult result = vkAcquireNextImageKHR(m_vulkanDevice->logicalDevice(), m_swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE, &m_imageIndex);
     if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }
}

VkResult VulkanSwapchain::endFrame(const VkSemaphore *signalSemaphores)
{
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapchain;
    presentInfo.pImageIndices = &m_imageIndex;
    if (signalSemaphores != nullptr) {
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
    }
    return vkQueuePresentKHR(m_presentQueue, &presentInfo);
}


void VulkanSwapchain::createSampleImage()
{
    m_sampleImage = new VulkanImage(m_vulkanDevice);
    m_sampleImage->createImage(m_swapchainExtent.width, m_swapchainExtent.height, 1, m_vulkanDevice->maxUsableSampleCount(), m_swapchainImageFormat, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
}

void VulkanSwapchain::createDepthImage()
{
    m_depthImage = new VulkanImage(m_vulkanDevice);
    m_depthImage->createImage(m_swapchainExtent.width, m_swapchainExtent.height, 1, m_vulkanDevice->maxUsableSampleCount(), m_vulkanDevice->findDepthFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

SwapChainSupportDetails VulkanSwapchain::querySwapChainSupport(VulkanDevice *device)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->physicalDevice(), device->surface(), &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device->physicalDevice(), device->surface(), &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device->physicalDevice(), device->surface(), &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device->physicalDevice(), device->surface(), &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device->physicalDevice(), device->surface(), &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR VulkanSwapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR VulkanSwapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_IMMEDIATE_KHR;
}

VkExtent2D VulkanSwapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    return capabilities.currentExtent; 
}