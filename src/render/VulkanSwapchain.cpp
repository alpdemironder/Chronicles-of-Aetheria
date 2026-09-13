#include "VulkanSwapchain.hpp"
#include "VulkanContext.hpp"
#include <iostream>
#include <algorithm>

namespace Aetheria {

VulkanSwapchain::VulkanSwapchain(VulkanContext* context, uint32_t width, uint32_t height)
    : context(context) {
    if (!createSwapchain(width, height)) return;
    if (!createImageViews()) return;
    if (!createRenderPass()) return;
    if (!createDepthResources()) return;
    if (!createFramebuffers()) return;

    initialized = true;
    std::cout << "VulkanSwapchain created successfully (" << extent.width << "x" << extent.height << ")" << std::endl;
}

VulkanSwapchain::~VulkanSwapchain() {
    cleanup();
}

void VulkanSwapchain::cleanup() {
    VkDevice device = context->getDevice();
    if (device) {
        vkDeviceWaitIdle(device);

        for (auto fb : framebuffers) {
            if (fb) vkDestroyFramebuffer(device, fb, nullptr);
        }
        framebuffers.clear();

        if (depthImageView) vkDestroyImageView(device, depthImageView, nullptr);
        if (depthImage) vkDestroyImage(device, depthImage, nullptr);
        if (depthImageMemory) vkFreeMemory(device, depthImageMemory, nullptr);
        depthImageView = 0; depthImage = 0; depthImageMemory = 0;

        if (renderPass) {
            vkDestroyRenderPass(device, renderPass, nullptr);
            renderPass = 0;
        }

        for (auto iv : swapchainImageViews) {
            if (iv) vkDestroyImageView(device, iv, nullptr);
        }
        swapchainImageViews.clear();

        if (swapchain) {
            vkDestroySwapchainKHR(device, swapchain, nullptr);
            swapchain = 0;
        }
    }
}

void VulkanSwapchain::recreate(uint32_t width, uint32_t height) {
    cleanup();
    createSwapchain(width, height);
    createImageViews();
    createRenderPass();
    createDepthResources();
    createFramebuffers();
}

bool VulkanSwapchain::createSwapchain(uint32_t width, uint32_t height) {
    VkPhysicalDevice physDev = context->getPhysicalDevice();
    VkSurfaceKHR surface = context->getSurface();

    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physDev, surface, &capabilities);

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }

    imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
    extent.width = std::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    extent.height = std::clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = imageFormat;
    createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = 1; // VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR; // Standard VSync
    createInfo.clipped = VK_TRUE;

    VkResult res = vkCreateSwapchainKHR(context->getDevice(), &createInfo, nullptr, &swapchain);
    if (res != VK_SUCCESS) {
        std::cerr << "Failed to create swapchain! Code: " << res << std::endl;
        return false;
    }

    uint32_t count = 0;
    vkGetSwapchainImagesKHR(context->getDevice(), swapchain, &count, nullptr);
    swapchainImages.resize(count);
    vkGetSwapchainImagesKHR(context->getDevice(), swapchain, &count, swapchainImages.data());

    return true;
}

bool VulkanSwapchain::createImageViews() {
    VkDevice device = context->getDevice();
    swapchainImageViews.resize(swapchainImages.size());

    for (size_t i = 0; i < swapchainImages.size(); ++i) {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapchainImages[i];
        createInfo.viewType = 1; // 2D
        createInfo.format = imageFormat;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &createInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS) {
            return false;
        }
    }
    return true;
}

bool VulkanSwapchain::createRenderPass() {
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = imageFormat;
    colorAttachment.samples = 1;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = depthFormat;
    depthAttachment.samples = 1;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkAttachmentDescription attachments[2] = { colorAttachment, depthAttachment };
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 2;
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    return vkCreateRenderPass(context->getDevice(), &renderPassInfo, nullptr, &renderPass) == VK_SUCCESS;
}

bool VulkanSwapchain::createDepthResources() {
    VkDevice device = context->getDevice();

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = 1; // 2D
    imageInfo.extent.width = extent.width;
    imageInfo.extent.height = extent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = depthFormat;
    imageInfo.tiling = 0; // OPTIMAL
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.samples = 1;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device, &imageInfo, nullptr, &depthImage) != VK_SUCCESS) return false;

    VkMemoryRequirements memReq;
    vkGetImageMemoryRequirements(device, depthImage, &memReq);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = context->findMemoryType(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &depthImageMemory) != VK_SUCCESS) return false;
    vkBindImageMemory(device, depthImage, depthImageMemory, 0);

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = depthImage;
    viewInfo.viewType = 1; // 2D
    viewInfo.format = depthFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    return vkCreateImageView(device, &viewInfo, nullptr, &depthImageView) == VK_SUCCESS;
}

bool VulkanSwapchain::createFramebuffers() {
    VkDevice device = context->getDevice();
    framebuffers.resize(swapchainImageViews.size());

    for (size_t i = 0; i < swapchainImageViews.size(); ++i) {
        VkImageView attachments[2] = {
            swapchainImageViews[i],
            depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 2;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
            return false;
        }
    }
    return true;
}

VkResult VulkanSwapchain::acquireNextImage(VkSemaphore semaphore, uint32_t* imageIndex) {
    return vkAcquireNextImageKHR(context->getDevice(), swapchain, UINT64_MAX, semaphore, 0, imageIndex);
}

VkResult VulkanSwapchain::present(VkSemaphore waitSemaphore, uint32_t imageIndex) {
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &waitSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &imageIndex;

    return vkQueuePresentKHR(context->getGraphicsQueue(), &presentInfo);
}

} // namespace Aetheria
