#include <random>
#define _USE_MATH_DEFINES
#include <math.h>
#include "DescriptorBufferApplication.h"
#include "VulkanShader.h"

#include "DescriptorBufferApplication.h"

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



DescriptorBufferApplication::DescriptorBufferApplication()
{
	m_mainWindow.setWindowTitle("Descriptor Buffer");
	m_camera.setPosition(glm::vec3(0, 0, -30.f));

	m_camera.setRotation(glm::vec3(0.0f, 15.0f, 0.0f));
	m_camera.setFieldOfView(60);
	m_camera.setAspectRatio((float)m_mainWindow.width() / m_mainWindow.height());
	m_camera.setNearPlane(0.1);
	m_camera.setFarPlane(256);
}

DescriptorBufferApplication::~DescriptorBufferApplication()
{
}

void DescriptorBufferApplication::prepare()
{
	loadAssets();
}

void DescriptorBufferApplication::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
}

void DescriptorBufferApplication::updateUniformBuffer(uint32_t currentImage)
{
}

void DescriptorBufferApplication::mouseEvent(MouseInfo& info)
{
	static glm::vec2 downOrigin(0, 0);
	float factor = 0.005;
	if (info.leftDown) {
		if (downOrigin == glm::vec2(0, 0)) downOrigin = info.m_mousePos;
		float dx = (info.m_mousePos.x - downOrigin.x) * factor;
		float dy = (info.m_mousePos.y - downOrigin.y) * factor;

		m_camera.rotate(glm::vec3(-dy, dx, 0));
		m_viewChanged = true;
	}
	else
	{
		downOrigin = glm::vec2(0, 0);
	}
}

void DescriptorBufferApplication::loadAssets()
{
	const uint32_t loadingFlags = GLTF::PreTransformVertices | GLTF::FlipY | GLTF::PreMultiplyVertexColors;

}

void DescriptorBufferApplication::createUniformBuffer()
{
	m_uniformViewBuffer = std::make_shared<VulkanBuffer>(*m_vulkanDevice, sizeof(uboVs), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	m_uniformViewBuffer->mapMemory();

}
