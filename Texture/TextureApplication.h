#pragma once

#include "Engine.h"
#include "Camera.h"


struct UniformContent
{
	glm::mat4 projection;
	glm::mat4 modelView;
	glm::vec4 viewPos;
	float lodBias = 0.0f;
};

struct Vertex {
	float pos[3];
	float uv[2];
	float normal[3];
};

class TextureApplication : public Engine {
public:
	TextureApplication();
	~TextureApplication();

	void prepare() override;
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void updateUniformBuffer(uint32_t currentImage);
private:
	void generateQuad();
	void createUniformBuffer();
	void loadTexture();
	void createLayouts();
	void setupDescriptor();
	void createGraphicPipeline();
private:
	Camera m_camera;
	UniformContent m_uniformContent;
	std::shared_ptr<VulkanImage> m_texture;
	std::shared_ptr<VulkanBuffer> m_indexBuffer;
	std::shared_ptr<VulkanBuffer> m_vertexBuffer;
	std::shared_ptr<VulkanBuffer> m_uniformBuffer;

	VkPipeline m_pipeline;
	VkPipelineLayout m_pipelineLayout;
	VkDescriptorSet m_descriptorSet;
	VkDescriptorSetLayout m_descriptorSetLayout;
	VkDescriptorPool m_descriptorPool;
	VkSampler m_textureSampler;
};