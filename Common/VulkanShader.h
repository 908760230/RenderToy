#pragma once

#include "vulkan/vulkan.h"
#include "VulkanDevice.h"
#include <fstream>
#include <vector>

class VulkanShader
{
public:
	VulkanShader(VulkanDevice *device, const std::string &path);
	~VulkanShader();

	VkShaderModule module() const { return m_module; };
private:
	std::vector<char> readFile(const std::string& filename);
	VkShaderModule createShaderModule(const std::vector<char>& code);
private:
	VkShaderModule m_module;
	VulkanDevice* m_vulkanDevice;
};

