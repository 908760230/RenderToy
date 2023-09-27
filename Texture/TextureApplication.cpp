#include "ktx.h"
#include "ktxvulkan.h"

#include "TextureApplication.h"
#include "VulkanShader.h"

TextureApplication::TextureApplication()
{
	m_mainWindow.setWindowTitle("sample_texture");
	m_mainWindow.resize(1280, 720);
	m_camera.setPosition(glm::vec3(0, 0, -2.5f));

	m_camera.setRotation(glm::vec3(0.0f, 15.0f, 0.0f));
	m_camera.setFieldOfView(60);
	m_camera.setAspectRatio((float)m_mainWindow.width() / m_mainWindow.height());
	m_camera.setNearPlane(0.1);
	m_camera.setFarPlane(256);
}

TextureApplication::~TextureApplication()
{
	vkDestroySampler(m_vulkanDevice->logicalDevice(), m_textureSampler, nullptr);

	vkDestroyPipelineLayout(m_vulkanDevice->logicalDevice(), m_pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(m_vulkanDevice->logicalDevice(), m_descriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(m_vulkanDevice->logicalDevice(), m_descriptorPool, nullptr);

	vkDestroyPipeline(m_vulkanDevice->logicalDevice(), m_pipeline, nullptr);

}

void TextureApplication::prepare()
{
	loadTexture();
	generateQuad();
	createUniformBuffer();
	createLayouts();
	setupDescriptor();
	createGraphicPipeline();
	createSyncObjects();
	buildCommandBuffers();
}

void TextureApplication::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_renderPass;
	renderPassInfo.framebuffer = m_framebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_swapchain->extend2D();

	std::array<VkClearValue, 2> clearValue{};
	clearValue[0].color = { 0, 0, 0, 1.0f };
	clearValue[1].depthStencil = { 1.0f,0 };

	renderPassInfo.clearValueCount = clearValue.size();
	renderPassInfo.pClearValues = clearValue.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)m_swapchain->extend2D().width;
	viewport.height = (float)m_swapchain->extend2D().height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_swapchain->extend2D();
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	VkBuffer vertexBuffers[] = { m_vertexBuffer->buffer() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->buffer(), 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);
	vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}

void TextureApplication::updateUniformBuffer(uint32_t currentImage)
{
	m_uniformContent.modelView = m_camera.getView();
	m_uniformContent.projection = m_camera.getProjection();
	m_uniformContent.viewPos = m_camera.getViewPos();

	memcpy(m_uniformBuffer->data(), &m_uniformContent, sizeof(m_uniformContent));
}

void TextureApplication::generateQuad()
{
	std::vector<Vertex> vertices =
	{
		{ {  1.0f,  1.0f, 0.0f }, { 1.0f, 1.0f },{ 0.0f, 0.0f, 1.0f } },
		{ { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f } },
		{ { -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } },
		{ {  1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } }
	};

	// Setup indices
	std::vector<uint32_t> indices = { 0,1,2, 2,3,0 };

	m_vertexBuffer = std::make_shared<VulkanBuffer>(*m_vulkanDevice, vertices.size() * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	m_vertexBuffer->mapMemory();
	m_vertexBuffer->update(vertices.data());

	m_indexBuffer = std::make_shared<VulkanBuffer>(*m_vulkanDevice, indices.size() * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	m_indexBuffer->mapMemory();
	m_indexBuffer->update(indices.data());
}

void TextureApplication::createUniformBuffer()
{
	m_uniformBuffer = std::make_shared<VulkanBuffer>(*m_vulkanDevice, sizeof(UniformContent), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	m_uniformBuffer->mapMemory();
	updateUniformBuffer(0);
	m_uniformBuffer->flushMemory();
}

void TextureApplication::loadTexture()
{
	ktxTexture *ktxTexture;
	ktxResult result = ktxTexture_CreateFromNamedFile("../../Texture/metalplate01_rgba.ktx",KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,&ktxTexture);
	if (result != KTX_SUCCESS) throw std::runtime_error("failed to load ktx texture");

	ktx_size_t textureSize = ktxTexture_GetSize(ktxTexture);
	uint32_t textureWidth = ktxTexture->baseWidth;
	uint32_t textureHeight = ktxTexture->baseHeight;
	uint32_t textureMipmaps = ktxTexture->numLevels;

	VulkanBuffer stagingBuffer(*m_vulkanDevice, textureSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	stagingBuffer.mapMemory();
	stagingBuffer.update(ktxTexture_GetData(ktxTexture));

	m_texture = std::make_shared<VulkanImage>(m_vulkanDevice);
	m_texture->createImage(textureWidth, textureHeight, textureMipmaps, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	
	VulkanCommand command(*m_vulkanDevice);
	command.transitionImageLayout(m_texture->image(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, textureMipmaps);

	VulkanCommand copyCommand(*m_vulkanDevice);
	copyCommand.copyBufferToImage(stagingBuffer.buffer(), m_texture->image(), textureWidth, textureHeight);

	m_texture->generateMipMaps(VK_FORMAT_R8G8B8A8_UNORM, textureWidth, textureHeight, textureMipmaps);


	VkSamplerCreateInfo samplerCreateInfo{};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.mipLodBias = 0;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerCreateInfo.minLod = 0;
	samplerCreateInfo.maxLod = textureMipmaps;
	if (m_vulkanDevice->features.samplerAnisotropy) {
		samplerCreateInfo.maxAnisotropy = m_vulkanDevice->properties.limits.maxSamplerAnisotropy;
		samplerCreateInfo.anisotropyEnable = VK_TRUE;
	}
	else {
		samplerCreateInfo.maxAnisotropy = 1;
		samplerCreateInfo.anisotropyEnable = VK_FALSE;
	}

	samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

	auto samperResult = vkCreateSampler(m_vulkanDevice->logicalDevice(), &samplerCreateInfo, nullptr, &m_textureSampler);
	if (samperResult != VK_SUCCESS) throw std::runtime_error("failed to create image sampler");
}

void TextureApplication::createLayouts()
{
	VkDescriptorSetLayoutBinding vertexShaderUniform{};
	vertexShaderUniform.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vertexShaderUniform.descriptorCount = 1;
	vertexShaderUniform.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderUniform.binding = 0;

	VkDescriptorSetLayoutBinding fragmentImageSampler{};
	fragmentImageSampler.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	fragmentImageSampler.descriptorCount = 1;
	fragmentImageSampler.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentImageSampler.binding = 1;

	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = { vertexShaderUniform,fragmentImageSampler };
	VkDescriptorSetLayoutCreateInfo setlayoutCreateInfo{};
	setlayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	setlayoutCreateInfo.bindingCount = setLayoutBindings.size();
	setlayoutCreateInfo.pBindings = setLayoutBindings.data();
	auto result = vkCreateDescriptorSetLayout(m_vulkanDevice->logicalDevice(), &setlayoutCreateInfo, nullptr, &m_descriptorSetLayout);
	if (result != VK_SUCCESS) throw std::runtime_error("failed to create descriptor set layout");

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &m_descriptorSetLayout;
	result = vkCreatePipelineLayout(m_vulkanDevice->logicalDevice(), &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout);
	if (result != VK_SUCCESS) throw std::runtime_error("failed to create pipeline layout");
}

void TextureApplication::setupDescriptor()
{
	VkDescriptorPoolSize uniformDescriptorPool{};
	uniformDescriptorPool.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformDescriptorPool.descriptorCount = 1;

	VkDescriptorPoolSize imageSamplerDescriptorPool{};
	imageSamplerDescriptorPool.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	imageSamplerDescriptorPool.descriptorCount = 1;

	std::vector<VkDescriptorPoolSize> poolSizes = { uniformDescriptorPool, imageSamplerDescriptorPool };
	VkDescriptorPoolCreateInfo descriptorPoolInfo{};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.poolSizeCount = poolSizes.size();
	descriptorPoolInfo.pPoolSizes = poolSizes.data();
	descriptorPoolInfo.maxSets = 2;

	auto result = vkCreateDescriptorPool(m_vulkanDevice->logicalDevice(), &descriptorPoolInfo, nullptr, &m_descriptorPool);
	if (result != VK_SUCCESS) throw std::runtime_error("failed to create descriptor pool");

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_descriptorPool;
	allocInfo.pSetLayouts = &m_descriptorSetLayout;
	allocInfo.descriptorSetCount = 1;

	result = vkAllocateDescriptorSets(m_vulkanDevice->logicalDevice(), &allocInfo, &m_descriptorSet);
	if (result != VK_SUCCESS) throw std::runtime_error("failed to allocate descriptor set");

	VkDescriptorImageInfo textureDescriptor{};
	textureDescriptor.imageView = m_texture->imageView();
	textureDescriptor.sampler = m_textureSampler;
	textureDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet uniformWriteDescriptorSet{};
	uniformWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uniformWriteDescriptorSet.dstSet = m_descriptorSet;
	uniformWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformWriteDescriptorSet.dstBinding = 0;
	uniformWriteDescriptorSet.pBufferInfo = &m_uniformBuffer->descriptor;
	uniformWriteDescriptorSet.descriptorCount = 1;

	VkWriteDescriptorSet textureWriteDescriptorSet{};
	textureWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	textureWriteDescriptorSet.dstSet = m_descriptorSet;
	textureWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	textureWriteDescriptorSet.dstBinding = 1;
	textureWriteDescriptorSet.pImageInfo = &textureDescriptor;
	textureWriteDescriptorSet.descriptorCount = 1;
	
	std::vector<VkWriteDescriptorSet> writeDescriptorSets = { uniformWriteDescriptorSet,textureWriteDescriptorSet };
	vkUpdateDescriptorSets(m_vulkanDevice->logicalDevice(), writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
}

void TextureApplication::createGraphicPipeline()
{
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	bindingDescription.stride = sizeof(Vertex);
	std::vector<VkVertexInputBindingDescription> bindingDescriptions = { bindingDescription };

	VkVertexInputAttributeDescription positonInputAttribute{};
	positonInputAttribute.binding = 0;
	positonInputAttribute.location = 0;
	positonInputAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	positonInputAttribute.offset = offsetof(Vertex, pos);

	VkVertexInputAttributeDescription texCoordinateInputAttribute{};
	texCoordinateInputAttribute.binding = 0;
	texCoordinateInputAttribute.location = 1;
	texCoordinateInputAttribute.format = VK_FORMAT_R32G32_SFLOAT;
	texCoordinateInputAttribute.offset = offsetof(Vertex, uv);

	VkVertexInputAttributeDescription normalInputAttribute{};
	normalInputAttribute.binding = 0;
	normalInputAttribute.location = 2;
	normalInputAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	normalInputAttribute.offset = offsetof(Vertex, normal);
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions = { positonInputAttribute,texCoordinateInputAttribute,normalInputAttribute };

	VkPipelineVertexInputStateCreateInfo vertexInputState{};
	vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputState.vertexBindingDescriptionCount = bindingDescriptions.size();
	vertexInputState.pVertexBindingDescriptions = bindingDescriptions.data();
	vertexInputState.vertexAttributeDescriptionCount = attributeDescriptions.size();
	vertexInputState.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
	inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyState.primitiveRestartEnable = VK_FALSE;
	inputAssemblyState.flags = 0;

	VkPipelineRasterizationStateCreateInfo rasterizationState{};
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.lineWidth = 1;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.cullMode = VK_CULL_MODE_NONE;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.flags = 0;
	rasterizationState.depthClampEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState blendAttachmentState{};
	blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	blendAttachmentState.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlendState{};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &blendAttachmentState;

	VkPipelineDepthStencilStateCreateInfo depthStencilState{};
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.depthTestEnable = VK_TRUE;
	depthStencilState.depthTestEnable = VK_TRUE;
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineMultisampleStateCreateInfo multisampleState{};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.rasterizationSamples = m_vulkanDevice->maxUsableSampleCount();

	std::vector<VkDynamicState> dynamicStateEnables{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = dynamicStateEnables.size();
	dynamicState.pDynamicStates = dynamicStateEnables.data();

	VulkanShader vertShder(m_vulkanDevice, "../../Texture/vert.spv");
	VulkanShader fragShder(m_vulkanDevice, "../../Texture/frag.spv");

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShder.module();
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShder.module();
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.layout = m_pipelineLayout;
	pipelineCreateInfo.renderPass = m_renderPass;
	pipelineCreateInfo.basePipelineIndex = -1;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.pVertexInputState = &vertexInputState;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	pipelineCreateInfo.pDynamicState = &dynamicState;
	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.pStages = shaderStages;

	auto result = vkCreateGraphicsPipelines(m_vulkanDevice->logicalDevice(), m_pipelineCache, 1, &pipelineCreateInfo, nullptr, &m_pipeline);
	if (result != VK_SUCCESS) throw std::runtime_error("failed to create graphic pipeline ");
}

void TextureApplication::mouseEvent(MouseInfo& info)
{
	static glm::vec2 downOrigin(0, 0);
	float factor = 0.005;
	if (info.leftDown) {
		if (downOrigin == glm::vec2(0, 0)) downOrigin = info.m_mousePos;
		float dx = (info.m_mousePos.x - downOrigin.x) * factor;
		float dy = (info.m_mousePos.y - downOrigin.y) * factor;

		m_camera.rotate(glm::vec3(-dy, dx, 0));
	}
	else
	{
		downOrigin = glm::vec2(0, 0);
	}
}

void TextureApplication::keyDown(size_t key)
{
	if (key == VK_UP) m_uniformContent.lodBias += 0.1;
	else if (key == VK_DOWN) m_uniformContent.lodBias -= 0.1;
}

