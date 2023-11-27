#pragma once

#include <VulkanDevice.h>
#include <VulkanImage.h>
#include <VulkanSwapchain.h>
#include <VulkanBuffer.h>
#include "stb_font_consolas_24_latin1.inl"
#include <vector>
#include <memory>

#define TEXTOVERLAY_MAX_CHAR_COUNT 2048

class TextOverlay
{
private:
	VulkanDevice *m_vulkanDevice = nullptr;
	VulkanSwapchain* m_swapchain = nullptr;
	VkQueue m_graphicQueue;
	float m_scale = 1.0f;

	VkSampler m_sampler;
	VulkanImage m_image;
	std::shared_ptr<VulkanBuffer> m_buffer;
	std::vector<VkCommandBuffer> m_cmdBuffers;

	VkDescriptorPool m_descriptorPool;
	VkDescriptorSetLayout m_descriptorSetLayout;
	VkDescriptorSet m_descriptorSet;
	VkPipelineLayout m_pipelineLayout;
	VkPipelineCache m_pipelineCache;
	VkPipeline m_pipeline;
	VkRenderPass m_renderPass;
	VkCommandPool m_commandPool;

	std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages;

	stb_fontchar m_stbFontData[STB_FONT_consolas_24_latin1_NUM_CHARS];
	uint32_t m_numLetters;
private:
	void prepareResources();
	void prepareRenderPass();
	void preparePipeline();

public:
	enum  TextAlign
	{
		AlignLeft,
		AlignRight,
		AlignCenter
	};
	bool visible = true;

	TextOverlay(VulkanDevice* device, VulkanSwapchain* swapchain);
};


