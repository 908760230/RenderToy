#pragma once

#include "Engine.h"
#include "UIClass.h"

class GUIApplication : public Engine 
{
public:
	GUIApplication();
	~GUIApplication();

	void update() override;
	void prepare() override;
	void initGui();
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) override;

protected:
	void createGraphicPipeline();
	void createDescriptorSetLayout();
	void createDescriptorPool();
	void createDescriptorSets();
	void updateUniformBuffer(uint32_t imageIndex) override;
	void drawFrame() override;
	void rebuildFrame() override;
private:
	std::shared_ptr<UIClass> m_ui;
	VkDescriptorSetLayout m_descriptorSetLayout;
	VkPipelineLayout m_graphicsPipelineLayout;
	VkPipeline m_graphicsPipeline;
	VkDescriptorPool m_descriptorPool;

	std::shared_ptr<VulkanBuffer> m_vertexBuffer;
	std::shared_ptr<VulkanBuffer> m_indexBuffer;
	std::vector<std::shared_ptr<VulkanBuffer>> m_uniformBuffers;
	std::vector<VkDescriptorSet> m_descriptorSets;
};

