#pragma once

#include "Engine.h"
#include "Camera.h"

class DynamicUniformBuffferApplication : public Engine{
public:
	DynamicUniformBuffferApplication();
	~DynamicUniformBuffferApplication();

	void prepare() override;
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) override;
	void createRenderPass() override;
	void createFrameBuffers() override;
	void updateUniformBuffer(uint32_t currentImage) override;
	void drawFrame() override;
	void mouseEvent(MouseInfo& info) override;

	void updateDynamicUniformBuffer(bool force = false);


	void generateCubes();
	void createUniformBuffer();
	void setupDescriptorSetLayout();
	void setupDescriptorSet();
	void preparePipeline();
private:
	Camera m_camera;
	std::shared_ptr<VulkanImage> m_texture;
	std::shared_ptr<VulkanBuffer> m_indexBuffer;
	std::shared_ptr<VulkanBuffer> m_vertexBuffer;

	std::shared_ptr<VulkanBuffer> m_uniformViewBuffer; // project and view
	std::shared_ptr<VulkanBuffer> m_uniformDynamicBuffer; // object matrix

	VkPipeline m_pipeline;
	VkPipelineLayout m_pipelineLayout;
	VkDescriptorSet m_descriptorSet;
	VkDescriptorSetLayout m_descriptorSetLayout;
	VkDescriptorPool m_descriptorPool;
	VkSampler m_textureSampler;

private:
	struct 
	{
		glm::mat4* model = nullptr;
	} uboDataDynamic;

	struct 
	{
		glm::mat4 projection;
		glm::mat4 view;
	} uboVs;

	constexpr static int OBJECT_INSTANCES = 125;

	glm::vec3 rotations[OBJECT_INSTANCES];
	glm::vec3 rotationSpeeds[OBJECT_INSTANCES];

	float animationTimer = 0.0f;
	bool m_viewChanged = false;
};