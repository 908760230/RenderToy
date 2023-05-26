#include "VulkanBuffer.h"
#include <stdexcept>
#include "VulkanCommand.h"

VulkanBuffer::VulkanBuffer(VulkanDevice* vulkanDevice): m_vulkanDevice(vulkanDevice)
{
}

VulkanBuffer::~VulkanBuffer()
{
    clear();
}

void VulkanBuffer::createBuffer(void* source, size_t size,VkBufferUsageFlags usage)
{
    clear();
    VulkanBuffer stagingBuffer(m_vulkanDevice);

    stagingBuffer.createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBuffer.mapMemory(source);

    createBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    copyBuffer(stagingBuffer);
}

void VulkanBuffer::createBufferWithoutCopy(void* data, size_t size)
{
    clear();
    createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    mapMemory(data);
}

void VulkanBuffer::createUniformBuffer(size_t size)
{
    clear();
    createBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    mapMemory(nullptr, false);
}

void VulkanBuffer::mapMemory(void* source, bool unMap)
{
    vkMapMemory(m_vulkanDevice->logicalDevice(), m_bufferMemory, 0, m_memorySize, 0, &m_data);
    if (unMap) {
        memcpy(m_data, source, m_bufferSize);
        vkUnmapMemory(m_vulkanDevice->logicalDevice(), m_bufferMemory);
    }
}

void VulkanBuffer::copyBuffer(const VulkanBuffer& other)
{
    VulkanCommand command(m_vulkanDevice);
    command.copyBuffer(other, *this);
}

void VulkanBuffer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
    clear();

    m_bufferSize = size;
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_vulkanDevice->logicalDevice(), &bufferInfo, nullptr, &m_buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_vulkanDevice->logicalDevice(), m_buffer, &memRequirements);

    m_memorySize = memRequirements.size;

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = m_vulkanDevice->findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_vulkanDevice->logicalDevice(), &allocInfo, nullptr, &m_bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(m_vulkanDevice->logicalDevice(), m_buffer, m_bufferMemory, 0);
}

void VulkanBuffer::clear()
{
    unMap();
    if (m_buffer) {
        vkDestroyBuffer(m_vulkanDevice->logicalDevice(), m_buffer, nullptr);
        m_buffer = nullptr;
    }
    if (m_bufferMemory) {
        vkFreeMemory(m_vulkanDevice->logicalDevice(), m_bufferMemory, nullptr);
        m_bufferMemory = nullptr;
    }
}

void VulkanBuffer::unMap()
{
    if (m_data)
    {
        vkUnmapMemory(m_vulkanDevice->logicalDevice(), m_bufferMemory);
        m_data = nullptr;
    }
}

void VulkanBuffer::flushMemory() {
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = m_bufferMemory;
    mappedRange.offset = 0;
    mappedRange.size = m_memorySize;
    vkFlushMappedMemoryRanges(m_vulkanDevice->logicalDevice(), 1, &mappedRange);
}
