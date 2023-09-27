#include "TextOverlay.h"
#include <stdexcept>
#include <glm/glm.hpp>
#include <VulkanCommand.h>

void TextOverlay::prepareResources()
{
	const uint32_t fontWidth = STB_FONT_consolas_24_latin1_BITMAP_WIDTH;
	const uint32_t fontHeight = STB_FONT_consolas_24_latin1_BITMAP_HEIGHT;

	static unsigned char font24pixels[fontHeight][fontWidth];
	stb_font_consolas_24_latin1(m_stbFontData, font24pixels, fontHeight);

	VkCommandPoolCreateInfo commandPoolInfo{};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.queueFamilyIndex = m_vulkanDevice->queueFamilyIndices()[0];
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	auto result = vkCreateCommandPool(m_vulkanDevice->logicalDevice(), &commandPoolInfo, nullptr, &m_commandPool);
	if (result != VK_SUCCESS) throw std::runtime_error("failed to create text command pool!");

	VkCommandBufferAllocateInfo commandBufferAllcoInfo{};
	commandBufferAllcoInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllcoInfo.commandPool = m_commandPool;
	commandBufferAllcoInfo.commandBufferCount = cmdBuffers.size();
	commandBufferAllcoInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	result = vkAllocateCommandBuffers(m_vulkanDevice->logicalDevice(), &commandBufferAllcoInfo, cmdBuffers.data());
	if(result!=VK_SUCCESS) throw std::runtime_error("failed to allocate text command buffers!");

	VkDeviceSize bufferSize = TEXTOVERLAY_MAX_CHAR_COUNT * sizeof(glm::vec4);
	m_buffer = std::make_shared<VulkanBuffer>(*m_vulkanDevice,bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	
	VulkanBuffer stagingBuffer(*m_vulkanDevice, fontWidth * fontHeight, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	stagingBuffer.mapMemory();
	stagingBuffer.update(&font24pixels[0][0]);

	m_image.setVulkanDevice(m_vulkanDevice);
	m_image.createImage(fontWidth, fontHeight, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

	VulkanCommand command(*m_vulkanDevice);
	command.transitionImageLayout(m_image.image(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);
	VulkanCommand copyCommand(*m_vulkanDevice);
	copyCommand.copyBufferToImage(stagingBuffer.buffer(), m_image.image(), fontWidth, fontWidth);
	VulkanCommand transitionCommand(*m_vulkanDevice);
	transitionCommand.transitionImageLayout(m_image.image(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.maxAnisotropy = 1.0f;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 1.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	result = vkCreateSampler(m_vulkanDevice->logicalDevice(), &samplerInfo, nullptr, &m_sampler);
	if (result != VK_SUCCESS) throw std::runtime_error("failed to create text sampler");

	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo descritorPoolInfo{};
	descritorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descritorPoolInfo.maxSets = 1;
	descritorPoolInfo.poolSizeCount = 1;
	descritorPoolInfo.pPoolSizes = &poolSize;
	result = vkCreateDescriptorPool(m_vulkanDevice->logicalDevice(), &descritorPoolInfo, nullptr, &m_descriptorPool);
	if (result != VK_SUCCESS) throw std::runtime_error("failed to create text Descriptor pool!");

	VkDescriptorSetLayoutBinding setLayoutBinding{};
	setLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	setLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	setLayoutBinding.binding = 0;
	setLayoutBinding.descriptorCount = 1;

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
	descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutInfo.bindingCount = 1;
	descriptorSetLayoutInfo.pBindings = &setLayoutBinding;

	result = vkCreateDescriptorSetLayout(m_vulkanDevice->logicalDevice(), &descriptorSetLayoutInfo, nullptr, &m_descriptorSetLayout);
	if (result != VK_SUCCESS) throw std::runtime_error("failed to create text descriptor setlayout!");

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
	result = vkCreatePipelineLayout(m_vulkanDevice->logicalDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout);
	if (result != VK_SUCCESS) throw std::runtime_error("failed to create text pipeline layout!");

	VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = m_descriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &m_descriptorSetLayout;
	result = vkAllocateDescriptorSets(m_vulkanDevice->logicalDevice(), &descriptorSetAllocInfo, &m_descriptorSet);
	if (result != VK_SUCCESS) throw std::runtime_error("failed to alloc text descritor sets!");

	VkDescriptorImageInfo descriptorImageInfo{};
	descriptorImageInfo.sampler = m_sampler;
	descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	descriptorImageInfo.imageView = m_image.imageView();

	VkWriteDescriptorSet writeDescriptorSet{};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.dstSet = m_descriptorSet;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDescriptorSet.pImageInfo = &descriptorImageInfo;
	writeDescriptorSet.dstBinding = 0;
}

void TextOverlay::prepareRenderPass()
{
}

void TextOverlay::preparePipeline()
{
}

TextOverlay::TextOverlay(VulkanDevice* device, VulkanSwapchain* swapchain) :m_vulkanDevice(device), m_swapchain(swapchain) {

};
