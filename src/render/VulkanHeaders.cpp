#include "VulkanHeaders.hpp"

namespace Aetheria {

// Global function pointers
PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = nullptr;
PFN_vkCreateInstance vkCreateInstance = nullptr;
PFN_vkDestroyInstance vkDestroyInstance = nullptr;
PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices = nullptr;
PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties = nullptr;
PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties = nullptr;
PFN_vkCreateDevice vkCreateDevice = nullptr;
PFN_vkDestroyDevice vkDestroyDevice = nullptr;
PFN_vkGetDeviceQueue vkGetDeviceQueue = nullptr;
PFN_vkDeviceWaitIdle vkDeviceWaitIdle = nullptr;

PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR = nullptr;
PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR = nullptr;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR = nullptr;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR = nullptr;
PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR = nullptr;
PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR = nullptr;
PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR = nullptr;
PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR = nullptr;
PFN_vkQueuePresentKHR vkQueuePresentKHR = nullptr;

PFN_vkCreateBuffer vkCreateBuffer = nullptr;
PFN_vkDestroyBuffer vkDestroyBuffer = nullptr;
PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements = nullptr;
PFN_vkAllocateMemory vkAllocateMemory = nullptr;
PFN_vkFreeMemory vkFreeMemory = nullptr;
PFN_vkBindBufferMemory vkBindBufferMemory = nullptr;
PFN_vkMapMemory vkMapMemory = nullptr;
PFN_vkUnmapMemory vkUnmapMemory = nullptr;
PFN_vkCreateImage vkCreateImage = nullptr;
PFN_vkDestroyImage vkDestroyImage = nullptr;
PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements = nullptr;
PFN_vkBindImageMemory vkBindImageMemory = nullptr;
PFN_vkCreateImageView vkCreateImageView = nullptr;
PFN_vkDestroyImageView vkDestroyImageView = nullptr;

PFN_vkCreateShaderModule vkCreateShaderModule = nullptr;
PFN_vkDestroyShaderModule vkDestroyShaderModule = nullptr;
PFN_vkCreatePipelineLayout vkCreatePipelineLayout = nullptr;
PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout = nullptr;
PFN_vkCreateRenderPass vkCreateRenderPass = nullptr;
PFN_vkDestroyRenderPass vkDestroyRenderPass = nullptr;
PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines = nullptr;
PFN_vkDestroyPipeline vkDestroyPipeline = nullptr;
PFN_vkCreateFramebuffer vkCreateFramebuffer = nullptr;
PFN_vkDestroyFramebuffer vkDestroyFramebuffer = nullptr;

PFN_vkCreateCommandPool vkCreateCommandPool = nullptr;
PFN_vkDestroyCommandPool vkDestroyCommandPool = nullptr;
PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers = nullptr;
PFN_vkFreeCommandBuffers vkFreeCommandBuffers = nullptr;
PFN_vkBeginCommandBuffer vkBeginCommandBuffer = nullptr;
PFN_vkEndCommandBuffer vkEndCommandBuffer = nullptr;
PFN_vkResetCommandBuffer vkResetCommandBuffer = nullptr;
PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass = nullptr;
PFN_vkCmdEndRenderPass vkCmdEndRenderPass = nullptr;
PFN_vkCmdBindPipeline vkCmdBindPipeline = nullptr;
PFN_vkCmdSetViewport vkCmdSetViewport = nullptr;
PFN_vkCmdSetScissor vkCmdSetScissor = nullptr;
PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers = nullptr;
PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer = nullptr;
PFN_vkCmdPushConstants vkCmdPushConstants = nullptr;
PFN_vkCmdDraw vkCmdDraw = nullptr;
PFN_vkCmdDrawIndexed vkCmdDrawIndexed = nullptr;
PFN_vkCmdCopyBuffer vkCmdCopyBuffer = nullptr;
PFN_vkCreateSemaphore vkCreateSemaphore = nullptr;
PFN_vkDestroySemaphore vkDestroySemaphore = nullptr;
PFN_vkCreateFence vkCreateFence = nullptr;
PFN_vkDestroyFence vkDestroyFence = nullptr;
PFN_vkWaitForFences vkWaitForFences = nullptr;
PFN_vkResetFences vkResetFences = nullptr;
PFN_vkQueueSubmit vkQueueSubmit = nullptr;

static HMODULE g_VulkanModule = nullptr;

bool loadVulkanLoader() {
    g_VulkanModule = LoadLibraryA("vulkan-1.dll");
    if (!g_VulkanModule) {
        std::cerr << "Failed to load vulkan-1.dll from system!" << std::endl;
        return false;
    }

    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)GetProcAddress(g_VulkanModule, "vkGetInstanceProcAddr");
    if (!vkGetInstanceProcAddr) {
        std::cerr << "Failed to get vkGetInstanceProcAddr!" << std::endl;
        return false;
    }

    vkCreateInstance = (PFN_vkCreateInstance)vkGetInstanceProcAddr(nullptr, "vkCreateInstance");
    return (vkCreateInstance != nullptr);
}

bool loadVulkanInstanceFunctions(VkInstance instance) {
    if (!instance) return false;

    #define LOAD_INST(fn) fn = (PFN_##fn)vkGetInstanceProcAddr(instance, #fn); if(!fn) return false;
    LOAD_INST(vkDestroyInstance);
    LOAD_INST(vkEnumeratePhysicalDevices);
    LOAD_INST(vkGetPhysicalDeviceQueueFamilyProperties);
    LOAD_INST(vkGetPhysicalDeviceMemoryProperties);
    LOAD_INST(vkCreateDevice);
    LOAD_INST(vkCreateWin32SurfaceKHR);
    LOAD_INST(vkDestroySurfaceKHR);
    LOAD_INST(vkGetPhysicalDeviceSurfaceSupportKHR);
    LOAD_INST(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    LOAD_INST(vkGetPhysicalDeviceSurfaceFormatsKHR);
    #undef LOAD_INST

    return true;
}

bool loadVulkanDeviceFunctions(VkDevice device) {
    if (!device) return false;

    auto getDeviceProc = (PFN_vkGetDeviceProcAddr)vkGetInstanceProcAddr(nullptr, "vkGetDeviceProcAddr");

    #define LOAD_DEV(fn) fn = (PFN_##fn)getDeviceProc(device, #fn); if(!fn) return false;
    LOAD_DEV(vkDestroyDevice);
    LOAD_DEV(vkGetDeviceQueue);
    LOAD_DEV(vkDeviceWaitIdle);

    LOAD_DEV(vkCreateSwapchainKHR);
    LOAD_DEV(vkDestroySwapchainKHR);
    LOAD_DEV(vkGetSwapchainImagesKHR);
    LOAD_DEV(vkAcquireNextImageKHR);
    LOAD_DEV(vkQueuePresentKHR);

    LOAD_DEV(vkCreateBuffer);
    LOAD_DEV(vkDestroyBuffer);
    LOAD_DEV(vkGetBufferMemoryRequirements);
    LOAD_DEV(vkAllocateMemory);
    LOAD_DEV(vkFreeMemory);
    LOAD_DEV(vkBindBufferMemory);
    LOAD_DEV(vkMapMemory);
    LOAD_DEV(vkUnmapMemory);

    LOAD_DEV(vkCreateImage);
    LOAD_DEV(vkDestroyImage);
    LOAD_DEV(vkGetImageMemoryRequirements);
    LOAD_DEV(vkBindImageMemory);
    LOAD_DEV(vkCreateImageView);
    LOAD_DEV(vkDestroyImageView);

    LOAD_DEV(vkCreateShaderModule);
    LOAD_DEV(vkDestroyShaderModule);
    LOAD_DEV(vkCreatePipelineLayout);
    LOAD_DEV(vkDestroyPipelineLayout);
    LOAD_DEV(vkCreateRenderPass);
    LOAD_DEV(vkDestroyRenderPass);
    LOAD_DEV(vkCreateGraphicsPipelines);
    LOAD_DEV(vkDestroyPipeline);
    LOAD_DEV(vkCreateFramebuffer);
    LOAD_DEV(vkDestroyFramebuffer);

    LOAD_DEV(vkCreateCommandPool);
    LOAD_DEV(vkDestroyCommandPool);
    LOAD_DEV(vkAllocateCommandBuffers);
    LOAD_DEV(vkFreeCommandBuffers);
    LOAD_DEV(vkBeginCommandBuffer);
    LOAD_DEV(vkEndCommandBuffer);
    LOAD_DEV(vkResetCommandBuffer);

    LOAD_DEV(vkCmdBeginRenderPass);
    LOAD_DEV(vkCmdEndRenderPass);
    LOAD_DEV(vkCmdBindPipeline);
    LOAD_DEV(vkCmdSetViewport);
    LOAD_DEV(vkCmdSetScissor);
    LOAD_DEV(vkCmdBindVertexBuffers);
    LOAD_DEV(vkCmdBindIndexBuffer);
    LOAD_DEV(vkCmdPushConstants);
    LOAD_DEV(vkCmdDraw);
    LOAD_DEV(vkCmdDrawIndexed);
    LOAD_DEV(vkCmdCopyBuffer);

    LOAD_DEV(vkCreateSemaphore);
    LOAD_DEV(vkDestroySemaphore);
    LOAD_DEV(vkCreateFence);
    LOAD_DEV(vkDestroyFence);
    LOAD_DEV(vkWaitForFences);
    LOAD_DEV(vkResetFences);
    LOAD_DEV(vkQueueSubmit);
    #undef LOAD_DEV

    return true;
}

} // namespace Aetheria
