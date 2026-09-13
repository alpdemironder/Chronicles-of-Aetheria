#pragma once

#include "VulkanHeaders.hpp"
#include <cstdint>

namespace Aetheria {

class VulkanContext;

class VulkanBuffer {
public:
    VulkanBuffer(VulkanContext* context, VkDeviceSize size, VkFlags usage, VkFlags properties);
    ~VulkanBuffer();

    VkBuffer getBuffer() const { return buffer; }
    VkDeviceMemory getMemory() const { return memory; }
    VkDeviceSize getSize() const { return size; }

    void uploadData(const void* data, VkDeviceSize uploadSize);
    void copyTo(VkBuffer dstBuffer, VkDeviceSize copySize);

    static VulkanBuffer* createDeviceLocal(VulkanContext* context, const void* data, VkDeviceSize size, VkFlags usage);

private:
    VulkanContext* context = nullptr;
    VkBuffer buffer = 0;
    VkDeviceMemory memory = 0;
    VkDeviceSize size = 0;
};

} // namespace Aetheria
