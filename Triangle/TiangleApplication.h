#pragma once
#include "Engine.h"
#include "VulkanBuffer.h"

struct  Vertex
{
	glm::vec3 pos;
	glm::vec3 color;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		return attributeDescriptions;
	}
	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color;
	}
};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1);
		}
	};
}




class TiangleApplication : public Engine
{
public:
    void prepare() override;

private:
    void createDescriptorSetLayout();
    void createGraphicPipeline();
	void createDescriptorPool();
	void createDescriptorSets();
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void updateUniformBuffer(uint32_t imageIndex) override;
private:
    VkDescriptorSetLayout m_descriptorSetLayout;
	VkPipelineLayout m_graphicsPipelineLayout;
	VkPipeline m_graphicsPipeline;
	VkDescriptorPool m_descriptorPool;

	std::shared_ptr<VulkanBuffer> m_vertexBuffer;
	std::shared_ptr<VulkanBuffer> m_indexBuffer;
	std::vector<std::shared_ptr<VulkanBuffer>> m_uniformBuffers;
	std::vector<VkDescriptorSet> m_descriptorSets;

};

