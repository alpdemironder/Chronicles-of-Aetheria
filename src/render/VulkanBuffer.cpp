#include "VulkanBuffer.hpp"
#include "VulkanContext.hpp"
#include <cstring>
#include <iostream>

namespace Aetheria {

VulkanBuffer::VulkanBuffer(VulkanContext* context, VkDeviceSize size, VkFlags usage, VkFlags properties)
    : context(context), size(size) {
    VkDevice device = context->getDevice();

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        std::cerr << "Failed to create VkBuffer!" << std::endl;
        return;
    }

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(device, buffer, &memReq);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = context->findMemoryType(memReq.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
        std::cerr << "Failed to allocate buffer memory!" << std::endl;
        return;
    }

    vkBindBufferMemory(device, buffer, memory, 0);
}

VulkanBuffer::~VulkanBuffer() {
    VkDevice device = context->getDevice();
    if (device) {
        if (buffer) vkDestroyBuffer(device, buffer, nullptr);
        if (memory) vkFreeMemory(device, memory, nullptr);
    }
}

void VulkanBuffer::uploadData(const void* data, VkDeviceSize uploadSize) {
    void* mapped = nullptr;
    vkMapMemory(context->getDevice(), memory, 0, uploadSize, 0, &mapped);
    std::memcpy(mapped, data, static_cast<size_t>(uploadSize));
    vkUnmapMemory(context->getDevice(), memory);
}

void VulkanBuffer::copyTo(VkBuffer dstBuffer, VkDeviceSize copySize) {
    VkCommandBuffer cmd = context->beginSingleTimeCommands();

    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = copySize;
    vkCmdCopyBuffer(cmd, buffer, dstBuffer, 1, &copyRegion);

    context->endSingleTimeCommands(cmd);
}

VulkanBuffer* VulkanBuffer::createDeviceLocal(VulkanContext* context, const void* data, VkDeviceSize size, VkFlags usage) {
    // 1. Create host visible staging buffer
    VulkanBuffer stagingBuffer(
        context, size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    stagingBuffer.uploadData(data, size);

    // 2. Create device local target buffer
    VulkanBuffer* deviceBuffer = new VulkanBuffer(
        context, size,
        usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    // 3. Copy staging to device local
    stagingBuffer.copyTo(deviceBuffer->getBuffer(), size);

    return deviceBuffer;
}

} // namespace Aetheria
