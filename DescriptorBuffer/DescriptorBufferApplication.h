#pragma once

#include "Engine.h"
#include "Camera.h"

#include "VulkanGLTF.h"
#include "Texture.h"

class DescriptorBufferApplication : public Engine {
public:
	DescriptorBufferApplication();
	~DescriptorBufferApplication();

	void prepare() override;
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) override;
	void updateUniformBuffer(uint32_t currentImage) override;
	void mouseEvent(MouseInfo& info) override;


	void loadAssets();

	void createUniformBuffer();
	void setupDescriptorSetLayout();
	void setupDescriptorSet();
	void preparePipeline();
private:
	
	Camera m_camera;
	struct Cube {
		glm::mat4 matrix;
		Texture2D texture;
		std::shared_ptr<VulkanBuffer> uniformBuffer;
		glm::vec3 rotation;
	};
	std::array<Cube, 2> cubes;


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


	float animationTimer = 0.0f;
	bool m_viewChanged = false;
};