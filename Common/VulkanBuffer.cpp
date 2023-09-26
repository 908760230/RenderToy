#include "VulkanBuffer.h"
#include <stdexcept>
#include "VulkanCommand.h"

VulkanBuffer::VulkanBuffer(VulkanDevice &vulkanDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties): 
    m_vulkanDevice(vulkanDevice),
    m_bufferSize(size),
    m_usage(usage)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(vulkanDevice.logicalDevice(), &bufferInfo, nullptr, &m_buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(vulkanDevice.logicalDevice(), m_buffer, &memRequirements);

    m_memorySize = memRequirements.size;

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = vulkanDevice.findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(vulkanDevice.logicalDevice(), &allocInfo, nullptr, &m_bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(vulkanDevice.logicalDevice(), m_buffer, m_bufferMemory, 0);

    descriptor.buffer = m_buffer;
    descriptor.offset = 0;
    descriptor.range = VK_WHOLE_SIZE;
}

VulkanBuffer::~VulkanBuffer()
{
    clear();
}

void VulkanBuffer::mapMemory()
{
    if (!m_mapped && m_bufferMemory) {
        vkMapMemory(m_vulkanDevice.logicalDevice(), m_bufferMemory, 0, m_memorySize, 0, &m_data);
        m_mapped = true;
    }
}

void VulkanBuffer::copyBuffer(const VulkanBuffer& other)
{
    VulkanCommand command(m_vulkanDevice);
    command.copyBuffer(other, *this);
}

void VulkanBuffer::clear()
{
    unMap();
    if (m_buffer) {
        vkDestroyBuffer(m_vulkanDevice.logicalDevice(), m_buffer, nullptr);
        m_buffer = nullptr;
    }
    if (m_bufferMemory) {
        vkFreeMemory(m_vulkanDevice.logicalDevice(), m_bufferMemory, nullptr);
        m_bufferMemory = nullptr;
    }
}

void VulkanBuffer::unMap()
{
    if (m_mapped && m_bufferMemory)
    {
        vkUnmapMemory(m_vulkanDevice.logicalDevice(), m_bufferMemory);
        m_mapped = false;
        m_data = nullptr;
    }
}

void VulkanBuffer::flushMemory() {
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = m_bufferMemory;
    mappedRange.offset = 0;
    mappedRange.size = m_memorySize;
    vkFlushMappedMemoryRanges(m_vulkanDevice.logicalDevice(), 1, &mappedRange);
}

void VulkanBuffer::update(void* source)
{
    memcpy(m_data, source, m_bufferSize);
    flushMemory();
}
