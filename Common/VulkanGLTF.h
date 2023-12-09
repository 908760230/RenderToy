#ifndef  VULKAN_TINYGLTF_H
#define VULKAN_TINYGLTF_H

#include <string>
#include <memory>
#include <vulkan/vulkan.h>

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "VulkanImage.h"


namespace GLTF {
	struct Node;

	enum DescriptorBindingFlags {
		ImageBaseColor = 0x00000001,
		ImageNormalMap = 0x00000002
	};

	enum FileLoadingFlags
	{
		None = 0x00000000,
		PreTransformVertices = 0x00000001,
		PreMultiplyVertexColors = 0x00000002,
		FlipY = 0x00000004,
		DontLoadImages=0x00000008
	};

	struct Dimensions
	{
		glm::vec3 min = glm::vec3(FLT_MAX);
		glm::vec3 max = glm::vec3(-FLT_MAX);
		glm::vec3 size;
		glm::vec3 center;
		float radius;
	};

	struct Texture
	{
		VulkanDevice* device = nullptr;
		VulkanImage* image = nullptr;
		VkImageLayout imageLayout;
		VkDeviceMemory deviceMemory;
		VkDescriptorImageInfo descriptor;
		VkSampler sampler;

		uint32_t width;
		uint32_t height;
		uint32_t mipLevels;
		uint32_t layoutCount;
		
		void updateDescriptor();
		void destroy();
		void fromGLTFImage(tinygltf::Image& gltfImage, const std::string& path, VulkanDevice* device, VkQueue copyQueue);
	private:
		bool isKtx(const std::string& url);
	};

	struct Material
	{
		enum AlphaMode
		{
			AM_OPAQUE,
			AM_MASK,
			AM_BLEND
		};
		AlphaMode alphaMode = AM_OPAQUE;
		float alphaCutOff = 1.f;
		float metallicFactor = 1.f;
		float roughnessFactor = 1.f;
		glm::vec4 baseColorFactor = glm::vec4(1);
		VulkanDevice* device = nullptr;
		
		Texture* baseColorTexture = nullptr;
		Texture* metallicRoughnessTexture = nullptr;
		Texture* normalTexture = nullptr;
		Texture* occlusionTexture = nullptr;
		Texture* emissiveTexture = nullptr;

		Texture* specularGlossinessTexture = nullptr;
		Texture* diffuseTexture = nullptr;

		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

		Material(VulkanDevice* device);
		void createDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, uint32_t descriptorBindingFlags);
	};

	struct Primitive {
		uint32_t firstIndex;
		uint32_t indexCount;
		uint32_t firstVertex;
		uint32_t vertexCount;
		Material& material;
		Dimensions dimensions;

		void setDimensions(glm::vec3 min, glm::vec3 max);
		Primitive(uint32_t firstIndex, uint32_t indexCount, Material& material) : firstIndex(firstIndex), indexCount(indexCount), material(material) {};
	};

	struct Mesh {
		VulkanDevice* device;
		std::vector<Primitive*> primitives;
		std::string name;

		std::shared_ptr<VulkanBuffer> uniformBuffer;
		VkDescriptorSet descriptorSet;
		struct UniformBlock
		{
			glm::mat4 matrix;
			glm::mat4 jointMatrix[64];
			float jointCount = 0;
		} uniformBlock;

		Mesh(VulkanDevice* device, glm::mat4 matrix);
		~Mesh();
 	};

	struct Skin
	{
		std::string name;
		Node* skeletonRoot = nullptr;
		std::vector<glm::mat4> inverseBindMatrices;
		std::vector<Node*> joints;
	};

	struct Node {
		Node* parent;
		uint32_t index;
		std::vector<Node*> children;
		glm::mat4 matrix;
		std::string name;
		Mesh* mesh;
		Skin* skin;
		int32_t skinIndex = -1;
		glm::vec3 translation;
		glm::vec3 scale;
		glm::quat rotation;
		glm::mat4 localMatrix();
		glm::mat4 getMatrix();
		
		void update();
		~Node();
	};

	struct AnimationChannel
	{
		enum PathType { TRANSLATION, ROTATION, SCALE };
		PathType path;
		Node* node;
		uint32_t samplerIndex;
	};

	struct AnimationSampler {
		enum InterpolationType { LINEAR, STEP, CUBICSPLINE };
		InterpolationType interpolation;
		std::vector<float> inputs;
		std::vector<glm::vec4> outputsVec4;
	};

	struct Animation {
		std::string name;
		std::vector<AnimationSampler> samplers;
		std::vector<AnimationChannel> channels;
		float start = std::numeric_limits<float>::max();
		float end = std::numeric_limits<float>::min();
	};

	enum class VertexComponent { Position, Normal, UV, Color, Tangent, Joint0, Weight0 };

	struct Vertex {
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 uv;
		glm::vec4 color;
		glm::vec4 joint0;
		glm::vec4 weight0;
		glm::vec4 tangent;
		static VkVertexInputBindingDescription vertexInputBindingDescription;
		static std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;
		static VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo;
		static VkVertexInputBindingDescription inputBindingDescription(uint32_t binding);
		static VkVertexInputAttributeDescription inputAttributeDescription(uint32_t binding, uint32_t location, VertexComponent component);
		static std::vector<VkVertexInputAttributeDescription> inputAttributeDescriptions(uint32_t binding, const std::vector<VertexComponent> components);
		/** @brief Returns the default pipeline vertex input state create info structure for the requested vertex components */
		static VkPipelineVertexInputStateCreateInfo* getPipelineVertexInputState(const std::vector<VertexComponent> components);
	};

	class Model {
	private:
		Texture* getTexture(uint32_t index);
		Texture emptyTexture;
		void createEmptyTexture(VkQueue tranferQueue);
	public:
		VulkanDevice* device;
		VkDescriptorPool descriptorPool;

		static VkDescriptorSetLayout descriptorSetLayoutImage;
		static VkDescriptorSetLayout descriptorSetLayoutUbo;
		static VkMemoryPropertyFlags memoryPropertyFlags;
		static uint32_t descriptorBindingFlags;

		std::shared_ptr<VulkanBuffer> verticesBuffer;
		std::shared_ptr<VulkanBuffer> indicesBuffer;
		size_t verticesCount;
		size_t indicesCount;

		std::vector<Node*> nodes;
		std::vector<Node*> linearNodes;
		std::vector<Skin*> skins;
		std::vector<Texture> textures;
		std::vector<Material> materials;
		std::vector<Animation> animations;

		Dimensions dimensions;
		bool metallicRoughnessWorkflow = true;
		bool buffersBound = false;
		std::string path;

		Model() {}
		~Model();

		void loadNode(Node* parent, const tinygltf::Node& node, uint32_t nodeIndex, const tinygltf::Model& model, std::vector<uint32_t>& indexBuffer, std::vector<Vertex>& vertexBuffer, float globalscale);
		void loadSkins(tinygltf::Model& gltfModel);
		void loadImages(tinygltf::Model& gltfModel, VulkanDevice* device, VkQueue transferQueue);
		void loadMaterials(tinygltf::Model& gltfModel);
		void loadAnimations(tinygltf::Model& gltfModel);
		void loadFromFile(std::string filename, VulkanDevice* device, VkQueue transferQueue, uint32_t fileLoadingFlags = FileLoadingFlags::None, float scale = 1.0f);
		void bindBuffers(VkCommandBuffer commandBuffer);
		void drawNode(Node* node, VkCommandBuffer commandBuffer, uint32_t renderFlags = 0, VkPipelineLayout pipelineLayout = VK_NULL_HANDLE, uint32_t bindImageSet = 1);
		void draw(VkCommandBuffer commandBuffer, uint32_t renderFlags = 0, VkPipelineLayout pipelineLayout = VK_NULL_HANDLE, uint32_t bindImageSet = 1);
		void getNodeDimensions(Node* node, glm::vec3& min, glm::vec3& max);
		void getSceneDimensions();
		void updateAnimation(uint32_t index, float time);
		Node* findNode(Node* parent, uint32_t index);
		Node* nodeFromIndex(uint32_t index);
		void prepareNodeDescriptor(Node* node, VkDescriptorSetLayout descriptorSetLayout);
	};

	enum RenderFlags {
		BindImages = 0x00000001,
		RenderOpaqueNodes = 0x00000002,
		RenderAlphaMaskedNodes = 0x00000004,
		RenderAlphaBlendedNodes = 0x00000008
	};
}



#endif // ! VULKAN_TINYGLTF_H
