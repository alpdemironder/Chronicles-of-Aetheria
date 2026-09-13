#pragma once

#include "VulkanHeaders.hpp"
#include <vector>

namespace Aetheria {

class VulkanContext;

class VulkanSwapchain {
public:
    VulkanSwapchain(VulkanContext* context, uint32_t width, uint32_t height);
    ~VulkanSwapchain();

    bool isValid() const { return initialized; }

    VkSwapchainKHR getSwapchain() const { return swapchain; }
    VkRenderPass getRenderPass() const { return renderPass; }
    VkFramebuffer getFramebuffer(uint32_t index) const { return framebuffers[index]; }
    VkExtent2D getExtent() const { return extent; }
    size_t getImageCount() const { return swapchainImages.size(); }

    VkResult acquireNextImage(VkSemaphore semaphore, uint32_t* imageIndex);
    VkResult present(VkSemaphore waitSemaphore, uint32_t imageIndex);

    void recreate(uint32_t width, uint32_t height);

private:
    void cleanup();
    bool createSwapchain(uint32_t width, uint32_t height);
    bool createImageViews();
    bool createRenderPass();
    bool createDepthResources();
    bool createFramebuffers();

    VulkanContext* context = nullptr;
    bool initialized = false;

    VkSwapchainKHR swapchain = 0;
    VkFormat imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
    VkExtent2D extent = {0, 0};

    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    std::vector<VkFramebuffer> framebuffers;

    VkRenderPass renderPass = 0;

    // Depth buffer
    VkImage depthImage = 0;
    VkDeviceMemory depthImageMemory = 0;
    VkImageView depthImageView = 0;
    VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
};

} // namespace Aetheria
