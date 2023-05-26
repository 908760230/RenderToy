#include "VulkanSwapchain.h"
#include "Engine.h"
#include <stdexcept>

VulkanSwapchain::VulkanSwapchain()
{
}

VulkanSwapchain::~VulkanSwapchain()
{
    clear();
}

void VulkanSwapchain::init(VulkanDevice* device)
{
    createSwapChain(device);
    createRenderPass();
    createSampleImage();
    createDepthImage();
    createFrameBuffers();
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

}

void VulkanSwapchain::clear()
{

    for (auto framebuffer : m_swapchainFramebuffers) {
        vkDestroyFramebuffer(m_vulkanDevice->logicalDevice(), framebuffer, nullptr);
    }
    if (m_depthImage) {
        delete m_depthImage;
        m_depthImage = nullptr;
    }
    if (m_sampleImage) {
        delete m_sampleImage;
        m_sampleImage = nullptr;
    }
    for (auto imageView : m_swapchainImageViews) vkDestroyImageView(m_vulkanDevice->logicalDevice(), imageView, nullptr);
    vkDestroyRenderPass(m_vulkanDevice->logicalDevice(), m_renderPass, nullptr);
    vkDestroySwapchainKHR(m_vulkanDevice->logicalDevice(), m_swapchain, nullptr);
}

void VulkanSwapchain::recreate()
{
    vkDeviceWaitIdle(m_vulkanDevice->logicalDevice());
    clear();
    init(m_vulkanDevice);
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

void VulkanSwapchain::createRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_swapchainImageFormat;
    colorAttachment.samples = m_vulkanDevice->maxUsableSampleCount();
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = m_vulkanDevice->findDepthFormat();
    depthAttachment.samples = m_vulkanDevice->maxUsableSampleCount();
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format = m_swapchainImageFormat;
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentResolveRef{};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.pResolveAttachments = &colorAttachmentResolveRef;

    std::array<VkSubpassDependency, 2> dependencies;
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    dependencies[0].dependencyFlags = 0;

    dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].dstSubpass = 0;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].srcAccessMask = 0;
    dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    dependencies[1].dependencyFlags = 0;

    std::vector<VkAttachmentDescription> attachmenst = { colorAttachment ,depthAttachment,colorAttachmentResolve };

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachmenst.size();
    renderPassInfo.pAttachments = attachmenst.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = dependencies.size();
    renderPassInfo.pDependencies = dependencies.data();

    if (vkCreateRenderPass(m_vulkanDevice->logicalDevice(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void VulkanSwapchain::createFrameBuffers()
{
    m_swapchainFramebuffers.resize(m_swapchainImages.size());

    for (size_t i = 0; i < m_swapchainImages.size(); i++) {
        std::vector<VkImageView> attachments = {
            m_sampleImage->imageView(),
            m_depthImage->imageView(),
            m_swapchainImageViews[i],
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = attachments.size();
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_swapchainExtent.width;
        framebufferInfo.height = m_swapchainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_vulkanDevice->logicalDevice(), &framebufferInfo, nullptr, &m_swapchainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
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