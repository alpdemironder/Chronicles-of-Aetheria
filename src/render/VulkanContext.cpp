#include "VulkanContext.hpp"
#include "../core/Window.hpp"
#include <iostream>

namespace Aetheria {

VulkanContext::VulkanContext(Window* window) : window(window) {
    if (!loadVulkanLoader()) {
        std::cerr << "CRITICAL: Could not load vulkan loader!" << std::endl;
        return;
    }

    if (!createInstance()) return;
    if (!createSurface(window)) return;
    if (!pickPhysicalDevice()) return;
    if (!createLogicalDevice()) return;
    if (!createCommandPool()) return;

    initialized = true;
    std::cout << "VulkanContext initialized successfully!" << std::endl;
}

VulkanContext::~VulkanContext() {
    if (device) {
        vkDeviceWaitIdle(device);
        if (commandPool) vkDestroyCommandPool(device, commandPool, nullptr);
        vkDestroyDevice(device, nullptr);
    }
    if (surface && instance) {
        vkDestroySurfaceKHR(instance, surface, nullptr);
    }
    if (instance) {
        vkDestroyInstance(instance, nullptr);
    }
}

bool VulkanContext::createInstance() {
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Aetheria RPG";
    appInfo.applicationVersion = 1;
    appInfo.pEngineName = "AetheriaEngine";
    appInfo.engineVersion = 1;
    appInfo.apiVersion = (1 << 22) | (1 << 12); // Vulkan 1.1

    const char* extensions[] = {
        "VK_KHR_surface",
        "VK_KHR_win32_surface"
    };

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = 2;
    createInfo.ppEnabledExtensionNames = extensions;

    VkResult res = vkCreateInstance(&createInfo, nullptr, &instance);
    if (res != VK_SUCCESS || !instance) {
        std::cerr << "Failed to create VkInstance! Code: " << res << std::endl;
        return false;
    }

    return loadVulkanInstanceFunctions(instance);
}

bool VulkanContext::createSurface(Window* win) {
    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = win->getHandle();
    createInfo.hinstance = win->getInstance();

    VkResult res = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface);
    if (res != VK_SUCCESS || !surface) {
        std::cerr << "Failed to create Win32 Surface! Code: " << res << std::endl;
        return false;
    }
    return true;
}

bool VulkanContext::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        std::cerr << "No physical GPUs with Vulkan support found!" << std::endl;
        return false;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& dev : devices) {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(dev, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(dev, &queueFamilyCount, queueFamilies.data());

        for (uint32_t i = 0; i < queueFamilyCount; ++i) {
            VkBool32 presentSupport = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, surface, &presentSupport);

            if ((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && presentSupport) {
                physicalDevice = dev;
                graphicsQueueFamily = i;
                vkGetPhysicalDeviceMemoryProperties(dev, &memProperties);
                return true;
            }
        }
    }

    std::cerr << "No suitable physical GPU found supporting graphics and presentation!" << std::endl;
    return false;
}

bool VulkanContext::createLogicalDevice() {
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = graphicsQueueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    const char* deviceExtensions[] = {
        "VK_KHR_swapchain"
    };

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.enabledExtensionCount = 1;
    createInfo.ppEnabledExtensionNames = deviceExtensions;

    VkResult res = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
    if (res != VK_SUCCESS || !device) {
        std::cerr << "Failed to create logical device! Code: " << res << std::endl;
        return false;
    }

    if (!loadVulkanDeviceFunctions(device)) {
        std::cerr << "Failed to load device functions!" << std::endl;
        return false;
    }

    vkGetDeviceQueue(device, graphicsQueueFamily, 0, &graphicsQueue);
    return true;
}

bool VulkanContext::createCommandPool() {
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = graphicsQueueFamily;
    poolInfo.flags = 2; // VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT

    return vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) == VK_SUCCESS;
}

uint32_t VulkanContext::findMemoryType(uint32_t typeFilter, VkFlags properties) const {
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    return 0;
}

VkCommandBuffer VulkanContext::beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = 0;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer = nullptr;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 1; // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void VulkanContext::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, 0);
    vkDeviceWaitIdle(device);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

} // namespace Aetheria
