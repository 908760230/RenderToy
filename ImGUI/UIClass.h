#pragma once
#include <imgui.h>
#include <array>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <VulkanBuffer.h>
#include <VulkanImage.h>
#include <Engine.h>

struct UISetting
{
	bool displayTriangle = true;
	std::array<float, 50> frameTimes{};
	float frameTimeMin = 999.f;
	float frameTimeMax = 0;
	float lightTimer = 0;
};

extern UISetting uiSettings;

struct PushConstBlock
{
	glm::vec2 scale;
	glm::vec2 translate;
};

class UIClass
{
private:
	ImGuiStyle m_vulkanStyle;
	int m_selectedStyle = 0;

	VkSampler m_sampler;
	VulkanBuffer m_vertexBuffer;
	VulkanBuffer m_indexBuffer;
	VulkanImage m_fontImage;
	VulkanDevice* m_vulkanDevice;

	VkPipelineLayout m_pipelineLayout;
	VkPipelineCache m_pipelineCache;
	VkPipeline m_pipeline;

	VkDescriptorPool m_descriptorPool;
	VkDescriptorSetLayout m_descriptorSetLayout;
	VkDescriptorSet m_descriptorSet;

	VkPhysicalDeviceDriverProperties m_driverProperties = {};
	PushConstBlock m_pushConstBlock;
	Engine* m_engine;
	int32_t m_vertexCount = 0;
	int32_t m_indexCount = 0;
public:
	UIClass(Engine* engine);
	~UIClass();

	void init(float width, float height);
	void setStyle(uint32_t index);
	void initResources(VkRenderPass renderPass);
	void newFrame(bool updateFrameGraph);
	void updateBuffers();
	void drawFrame(VkCommandBuffer commandBuffer);
};

