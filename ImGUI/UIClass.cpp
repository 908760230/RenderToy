#include "UIClass.h"
#include <VulkanCommand.h>
#include <VulkanBuffer.h>
#include <VulkanShader.h>

UISetting uiSettings;

UIClass::UIClass(Engine* engine)
{
	m_engine = engine;
	m_vulkanDevice = engine->vulkanDevice();

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = 1.0f;
	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(1.0f);
	m_fontImage.setVulkanDevice(m_vulkanDevice);
}

UIClass::~UIClass()
{
	ImGui::DestroyContext();
	vkDestroySampler(m_vulkanDevice->logicalDevice(), m_sampler, nullptr);
	vkDestroyPipelineCache(m_vulkanDevice->logicalDevice(), m_pipelineCache, nullptr);
	vkDestroyPipeline(m_vulkanDevice->logicalDevice(), m_pipeline, nullptr);
	vkDestroyPipelineLayout(m_vulkanDevice->logicalDevice(), m_pipelineLayout, nullptr);
	vkFreeDescriptorSets(m_vulkanDevice->logicalDevice(), m_descriptorPool, 1, &m_descriptorSet);
	vkDestroyDescriptorPool(m_vulkanDevice->logicalDevice(), m_descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(m_vulkanDevice->logicalDevice(), m_descriptorSetLayout, nullptr);
}

void UIClass::init(float width, float height)
{
	m_vulkanStyle = ImGui::GetStyle();
	m_vulkanStyle.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
	m_vulkanStyle.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
	m_vulkanStyle.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	m_vulkanStyle.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	m_vulkanStyle.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

	setStyle(0);
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(width, height);
	io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
	io.KeyMap[ImGuiKey_Tab] = VK_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
	io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
	io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
	io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
	io.KeyMap[ImGuiKey_Space] = VK_SPACE;
	io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
	
}

void UIClass::setStyle(uint32_t index)
{
	switch (index)
	{
	case 0:
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style = m_vulkanStyle;
		break;
	}
	case 1:
		ImGui::StyleColorsClassic();
		break;
	case 2:
		ImGui::StyleColorsDark();
		break;
	case 3:
		ImGui::StyleColorsLight();
		break;
	}
}

void UIClass::initResources(VkRenderPass renderPass)
{
	ImGuiIO& io = ImGui::GetIO();

	unsigned char* fontData;
	int texWidth, texHeight;
	io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
	VkDeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);


	// get driver's information
	VkPhysicalDeviceProperties2 deviceProperties2 = {};
	deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	deviceProperties2.pNext = &m_driverProperties;
	m_driverProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;
	vkGetPhysicalDeviceProperties2(m_vulkanDevice->physicalDevice(), &deviceProperties2);

	VulkanBuffer stagingBuffer(*m_vulkanDevice, uploadSize,VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	stagingBuffer.mapMemory();
	stagingBuffer.update(fontData);

	m_fontImage.createImage((uint32_t)texWidth, (uint32_t)texHeight, 1, VK_SAMPLE_COUNT_1_BIT,
		VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

	VulkanCommand command(*m_vulkanDevice);
	command.transitionImageLayout(m_fontImage.image(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);
	VulkanCommand copyCommand(*m_vulkanDevice);
	copyCommand.copyBufferToImage(stagingBuffer.buffer(), m_fontImage.image(), texWidth, texHeight);

	VulkanCommand transitionCommand(*m_vulkanDevice);
	transitionCommand.transitionImageLayout(m_fontImage.image(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.maxAnisotropy = 1.0f;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	vkCreateSampler(m_vulkanDevice->logicalDevice(), &samplerInfo, nullptr, &m_sampler);

	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo descriptorPoolInfo{};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.poolSizeCount =1;
	descriptorPoolInfo.pPoolSizes = &poolSize;
	descriptorPoolInfo.maxSets = 2;

	vkCreateDescriptorPool(m_vulkanDevice->logicalDevice(), &descriptorPoolInfo, nullptr, &m_descriptorPool);

	VkDescriptorSetLayoutBinding setlayoyBindings{};
	setlayoyBindings.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	setlayoyBindings.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	setlayoyBindings.binding = 0;
	setlayoyBindings.descriptorCount = 1;

	VkDescriptorSetLayoutCreateInfo descriptorLayout{};
	descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayout.bindingCount = 1;
	descriptorLayout.pBindings = &setlayoyBindings;
	vkCreateDescriptorSetLayout(m_vulkanDevice->logicalDevice(), &descriptorLayout, nullptr, &m_descriptorSetLayout);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_descriptorPool;
	allocInfo.pSetLayouts = &m_descriptorSetLayout;
	allocInfo.descriptorSetCount = 1;
	vkAllocateDescriptorSets(m_vulkanDevice->logicalDevice(), &allocInfo, &m_descriptorSet);

	VkDescriptorImageInfo fontDescriptor{};
	fontDescriptor.sampler = m_sampler;
	fontDescriptor.imageView = m_fontImage.imageView();
	fontDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet writeDescriptorSet{};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.dstSet = m_descriptorSet;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDescriptorSet.dstBinding = 0;
	writeDescriptorSet.pImageInfo = &fontDescriptor;
	writeDescriptorSet.descriptorCount = 1;
	vkUpdateDescriptorSets(m_vulkanDevice->logicalDevice(), 1, &writeDescriptorSet, 0, nullptr);

	VkPipelineCacheCreateInfo pipelineCacheCreateInfo{};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	vkCreatePipelineCache(m_vulkanDevice->logicalDevice(), &pipelineCacheCreateInfo, nullptr, &m_pipelineCache);

	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRange.size = sizeof(PushConstBlock);
	pushConstantRange.offset = 0;

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &m_descriptorSetLayout;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	vkCreatePipelineLayout(m_vulkanDevice->logicalDevice(), &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout);

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
	inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyState.flags = 0;
	inputAssemblyState.primitiveRestartEnable = VK_FALSE;

	VkPipelineRasterizationStateCreateInfo rasterizationState{}; 
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.cullMode = VK_CULL_MODE_NONE;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.flags = 0;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.lineWidth = 1.0f; 

	// Enable blending
	VkPipelineColorBlendAttachmentState blendAttachmentState{};
	blendAttachmentState.blendEnable = VK_TRUE;
	blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlendState{};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &blendAttachmentState;

	VkPipelineDepthStencilStateCreateInfo depthStencilState{};
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.depthTestEnable = VK_FALSE;
	depthStencilState.depthWriteEnable = VK_FALSE;
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
	viewportState.flags = 0;

	VkPipelineMultisampleStateCreateInfo multisampleState{};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.rasterizationSamples = m_vulkanDevice->maxUsableSampleCount();
	multisampleState.flags = 0;

	std::vector<VkDynamicState> dynamicStateEnables = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates = dynamicStateEnables.data();
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
	dynamicState.flags = 0; 

	VulkanShader vertShder(m_vulkanDevice, "../../ImGUI/ui.vert.spv");
	VulkanShader fragShder(m_vulkanDevice, "../../ImGUI/ui.frag.spv");

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

	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{ vertShaderStageInfo ,fragShaderStageInfo };

	VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.layout = m_pipelineLayout;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.flags = 0;
	pipelineCreateInfo.basePipelineIndex = -1;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	pipelineCreateInfo.pDynamicState = &dynamicState;
	pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCreateInfo.pStages = shaderStages.data();

	// Vertex bindings an attributes based on ImGui vertex definition
	VkVertexInputBindingDescription vertexInputBindings{};
	vertexInputBindings.binding = 0;
	vertexInputBindings.stride = sizeof(ImDrawVert);
	vertexInputBindings.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; 

	VkVertexInputAttributeDescription posAttribDescription{};
	posAttribDescription.binding = 0;
	posAttribDescription.location = 0;
	posAttribDescription.format = VK_FORMAT_R32G32_SFLOAT;
	posAttribDescription.offset = offsetof(ImDrawVert, pos);

	VkVertexInputAttributeDescription uvAttribDescription{};
	uvAttribDescription.binding = 0;
	uvAttribDescription.location = 1;
	uvAttribDescription.format = VK_FORMAT_R32G32_SFLOAT;
	uvAttribDescription.offset = offsetof(ImDrawVert, uv);

	VkVertexInputAttributeDescription colorAttribDescription{};
	colorAttribDescription.binding = 0;
	colorAttribDescription.location = 2;
	colorAttribDescription.format = VK_FORMAT_R8G8B8A8_UNORM;
	colorAttribDescription.offset = offsetof(ImDrawVert, col);

	std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = { posAttribDescription ,uvAttribDescription, colorAttribDescription };

	VkPipelineVertexInputStateCreateInfo vertexInputState{};
	vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputState.vertexBindingDescriptionCount = 1;
	vertexInputState.pVertexBindingDescriptions = &vertexInputBindings;
	vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
	vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes.data();

	pipelineCreateInfo.pVertexInputState = &vertexInputState;

    vkCreateGraphicsPipelines(m_vulkanDevice->logicalDevice(), m_pipelineCache, 1, &pipelineCreateInfo, nullptr, &m_pipeline);
}

void UIClass::newFrame(bool updateFrameGraph)
{
	ImGui::NewFrame();

	// Init imGui windows and elements

	// Debug window
	ImGui::SetWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	ImGui::SetWindowSize(ImVec2(300 , 300), ImGuiCond_Always);
	ImGui::TextUnformatted(m_engine->getWindowTitle().c_str());
	ImGui::TextUnformatted(m_vulkanDevice->properties.deviceName);

	//SRS - Display Vulkan API version and device driver information if available (otherwise blank)
	ImGui::Text("Vulkan API %i.%i.%i", VK_API_VERSION_MAJOR(m_vulkanDevice->properties.apiVersion), VK_API_VERSION_MINOR(m_vulkanDevice->properties.apiVersion), VK_API_VERSION_PATCH(m_vulkanDevice->properties.apiVersion));
	ImGui::Text("%s %s", m_driverProperties.driverName, m_driverProperties.driverInfo);

	// Update frame time display
	if (updateFrameGraph) {
		std::rotate(uiSettings.frameTimes.begin(), uiSettings.frameTimes.begin() + 1, uiSettings.frameTimes.end());
		float frameTime = 1000.0f / (m_engine->frameTimer * 1000.0f);
		uiSettings.frameTimes.back() = frameTime;
		if (frameTime < uiSettings.frameTimeMin) {
			uiSettings.frameTimeMin = frameTime;
		}
		if (frameTime > uiSettings.frameTimeMax) {
			uiSettings.frameTimeMax = frameTime;
		}
	}

	ImGui::PlotLines("Frame Times", &uiSettings.frameTimes[0], 50, 0, "", uiSettings.frameTimeMin, uiSettings.frameTimeMax, ImVec2(0, 80));


	// Example settings window
	ImGui::SetNextWindowPos(ImVec2(20 , 360 ), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300 , 200 ), ImGuiCond_FirstUseEver);
	ImGui::Begin("Example settings");
	ImGui::Checkbox("Display Triangle", &uiSettings.displayTriangle);
	//ImGui::ShowStyleSelector("UI style");
	if (ImGui::Combo("UI style", &m_selectedStyle, "Vulkan\0Classic\0Dark\0Light\0")) {
		setStyle(m_selectedStyle);
	}

	ImGui::End();

	//SRS - ShowDemoWindow() sets its own initial position and size, cannot override here
	ImGui::ShowDemoWindow();

	// Render to generate draw buffers
	ImGui::Render();
}

void UIClass::updateBuffers()
{
	ImDrawData* imDrawData = ImGui::GetDrawData();

	// Note: Alignment is done inside buffer creation
	VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
	VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

	if ((vertexBufferSize == 0) || (indexBufferSize == 0)) {
		return;
	}

	// Update buffers only if vertex or index count has been changed compared to current buffer size

	// Vertex buffer
	if (!m_vertexBuffer || (m_vertexCount != imDrawData->TotalVtxCount)) {
		vkDeviceWaitIdle(m_vulkanDevice->logicalDevice());
		m_vertexBuffer = std::make_shared<VulkanBuffer>(*m_vulkanDevice, vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		m_vertexBuffer->mapMemory();
		m_vertexCount = imDrawData->TotalVtxCount;
	}

	//// Index buffer
	if (!m_indexBuffer || (m_indexCount < imDrawData->TotalIdxCount)) {
		vkDeviceWaitIdle(m_vulkanDevice->logicalDevice());
		m_indexBuffer = std::make_shared<VulkanBuffer>(*m_vulkanDevice, indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		m_indexBuffer->mapMemory();
		m_indexCount = imDrawData->TotalIdxCount;
	}

	// Upload data
	ImDrawVert* vtxDst = (ImDrawVert*)m_vertexBuffer->data();
	ImDrawIdx* idxDst = (ImDrawIdx*)m_indexBuffer->data();

	for (int n = 0; n < imDrawData->CmdListsCount; n++) {
		const ImDrawList* cmd_list = imDrawData->CmdLists[n];
		memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
		memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
		vtxDst += cmd_list->VtxBuffer.Size;
		idxDst += cmd_list->IdxBuffer.Size;
	}

	// Flush to make writes visible to GPU
	m_vertexBuffer->flushMemory();
	m_indexBuffer->flushMemory();
}

void UIClass::drawFrame(VkCommandBuffer commandBuffer)
{
	ImGuiIO& io = ImGui::GetIO();

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

	VkViewport viewport{};
	viewport.width = ImGui::GetIO().DisplaySize.x;
	viewport.height = ImGui::GetIO().DisplaySize.y;
	viewport.minDepth = 0;
	viewport.maxDepth = 1;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	// UI scale and translate via push constants
	m_pushConstBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
	m_pushConstBlock.translate = glm::vec2(-1.0f);
	vkCmdPushConstants(commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstBlock), &m_pushConstBlock);

	// Render commands
	ImDrawData* imDrawData = ImGui::GetDrawData();
	int32_t vertexOffset = 0;
	int32_t indexOffset = 0;

	if (imDrawData->CmdListsCount > 0) {

		VkDeviceSize offsets[1] = { 0 };
		VkBuffer buffer = m_vertexBuffer->buffer();
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffer, offsets);
		vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->buffer(), 0, VK_INDEX_TYPE_UINT16);

		for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
		{
			const ImDrawList* cmd_list = imDrawData->CmdLists[i];
			for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
			{
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
				VkRect2D scissorRect;
				scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x), 0);
				scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y), 0);
				scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
				scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
				vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);
				vkCmdDrawIndexed(commandBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
				indexOffset += pcmd->ElemCount;
			}
			vertexOffset += cmd_list->VtxBuffer.Size;
		}
	}
}
