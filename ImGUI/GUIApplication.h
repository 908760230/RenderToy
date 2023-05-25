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

private:
	UIClass* m_ui = nullptr;
	VkPipelineLayout m_graphicsPipelineLayout;
	VkPipeline m_graphicsPipeline;

	VulkanBuffer m_vertexBuffer;
	VulkanBuffer m_indexBuffer;
};

