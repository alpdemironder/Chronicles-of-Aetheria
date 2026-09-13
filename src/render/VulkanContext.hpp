#pragma once

#include "VulkanHeaders.hpp"
#include <vector>
#include <string>

namespace Aetheria {

class Window;

class VulkanContext {
public:
    VulkanContext(Window* window);
    ~VulkanContext();

    bool isValid() const { return initialized; }

    VkInstance getInstance() const { return instance; }
    VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
    VkDevice getDevice() const { return device; }
    VkSurfaceKHR getSurface() const { return surface; }
    VkQueue getGraphicsQueue() const { return graphicsQueue; }
    uint32_t getGraphicsQueueFamily() const { return graphicsQueueFamily; }
    VkCommandPool getCommandPool() const { return commandPool; }

    uint32_t findMemoryType(uint32_t typeFilter, VkFlags properties) const;

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

private:
    bool createInstance();
    bool createSurface(Window* window);
    bool pickPhysicalDevice();
    bool createLogicalDevice();
    bool createCommandPool();

    bool initialized = false;
    Window* window = nullptr;

    VkInstance instance = nullptr;
    VkSurfaceKHR surface = 0;
    VkPhysicalDevice physicalDevice = nullptr;
    VkDevice device = nullptr;
    VkQueue graphicsQueue = nullptr;
    uint32_t graphicsQueueFamily = 0;
    VkCommandPool commandPool = 0;

    VkPhysicalDeviceMemoryProperties memProperties{};
};

} // namespace Aetheria
