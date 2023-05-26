#pragma once

#include <Engine.h>
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
	std::string getWindowTitle() const override { return "ImGui Demo"; }
private:
	UIClass* m_ui = nullptr;
	VkDescriptorSetLayout m_descriptorSetLayout;
	VkPipelineLayout m_graphicsPipelineLayout;
	VkPipeline m_graphicsPipeline;
	VkDescriptorPool m_descriptorPool;

	VulkanBuffer m_vertexBuffer;
	VulkanBuffer m_indexBuffer;
	std::vector<VulkanBuffer> m_uniformBuffers;
	std::vector<VkDescriptorSet> m_descriptorSets;
};

