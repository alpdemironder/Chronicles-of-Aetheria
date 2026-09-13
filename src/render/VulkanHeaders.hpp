#pragma once

#include <windows.h>
#include <cstdint>
#include <cstddef>
#include <iostream>

#define VKAPI_PTR __stdcall

namespace Aetheria {

// Vulkan Enums & Constants
using VkFlags = uint32_t;
using VkBool32 = uint32_t;
using VkDeviceSize = uint64_t;

#define VK_FALSE 0U
#define VK_TRUE 1U

enum VkResult {
    VK_SUCCESS = 0,
    VK_NOT_READY = 1,
    VK_TIMEOUT = 2,
    VK_EVENT_SET = 3,
    VK_EVENT_RESET = 4,
    VK_INCOMPLETE = 5,
    VK_ERROR_OUT_OF_HOST_MEMORY = -1,
    VK_ERROR_OUT_OF_DEVICE_MEMORY = -2,
    VK_ERROR_INITIALIZATION_FAILED = -3,
    VK_ERROR_DEVICE_LOST = -4,
    VK_ERROR_MEMORY_MAP_FAILED = -5,
    VK_ERROR_LAYER_NOT_PRESENT = -6,
    VK_ERROR_EXTENSION_NOT_PRESENT = -7,
    VK_ERROR_FEATURE_NOT_PRESENT = -8,
    VK_ERROR_INCOMPATIBLE_DRIVER = -9,
    VK_ERROR_TOO_MANY_OBJECTS = -10,
    VK_ERROR_FORMAT_NOT_SUPPORTED = -11,
    VK_ERROR_SURFACE_LOST_KHR = -1000000000,
    VK_SUBOPTIMAL_KHR = 1000001003,
    VK_ERROR_OUT_OF_DATE_KHR = -1000001004,
};

enum VkStructureType {
    VK_STRUCTURE_TYPE_APPLICATION_INFO = 0,
    VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO = 1,
    VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO = 2,
    VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO = 3,
    VK_STRUCTURE_TYPE_SUBMIT_INFO = 4,
    VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO = 5,
    VK_STRUCTURE_TYPE_FENCE_CREATE_INFO = 8,
    VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO = 9,
    VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO = 12,
    VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO = 14,
    VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO = 15,
    VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO = 16,
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO = 18,
    VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO = 19,
    VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO = 20,
    VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO = 22,
    VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO = 23,
    VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO = 24,
    VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO = 25,
    VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO = 26,
    VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO = 27,
    VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO = 28,
    VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO = 31,
    VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO = 38,
    VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO = 39,
    VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO = 40,
    VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO = 42,
    VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO = 43,
    VK_STRUCTURE_TYPE_BUFFER_COPY = 44,
    VK_STRUCTURE_TYPE_IMAGE_COPY = 45,
    VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER = 47,
    VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO = 28, // Fix: 28 is layout, 34 is graphics
    VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO_REAL = 34,
    VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO = 37,
    VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR = 1000009000,
    VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR = 1000001000,
    VK_STRUCTURE_TYPE_PRESENT_INFO_KHR = 1000001001,
};

enum VkFormat {
    VK_FORMAT_UNDEFINED = 0,
    VK_FORMAT_R8G8B8A8_UNORM = 37,
    VK_FORMAT_B8G8R8A8_UNORM = 44,
    VK_FORMAT_R32G32_SFLOAT = 103,
    VK_FORMAT_R32G32B32_SFLOAT = 106,
    VK_FORMAT_R32G32B32A32_SFLOAT = 109,
    VK_FORMAT_D32_SFLOAT = 126,
    VK_FORMAT_D24_UNORM_S8_UINT = 129,
};

enum VkColorSpaceKHR {
    VK_COLOR_SPACE_SRGB_NONLINEAR_KHR = 0,
};

enum VkPresentModeKHR {
    VK_PRESENT_MODE_IMMEDIATE_KHR = 0,
    VK_PRESENT_MODE_MAILBOX_KHR = 1,
    VK_PRESENT_MODE_FIFO_KHR = 2,
    VK_PRESENT_MODE_FIFO_RELAXED_KHR = 3,
};

enum VkSharingMode {
    VK_SHARING_MODE_EXCLUSIVE = 0,
    VK_SHARING_MODE_CONCURRENT = 1,
};

enum VkImageLayout {
    VK_IMAGE_LAYOUT_UNDEFINED = 0,
    VK_IMAGE_LAYOUT_GENERAL = 1,
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL = 2,
    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL = 3,
    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL = 5,
    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL = 6,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL = 7,
    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR = 1000001002,
};

enum VkAttachmentLoadOp {
    VK_ATTACHMENT_LOAD_OP_LOAD = 0,
    VK_ATTACHMENT_LOAD_OP_CLEAR = 1,
    VK_ATTACHMENT_LOAD_OP_DONT_CARE = 2,
};

enum VkAttachmentStoreOp {
    VK_ATTACHMENT_STORE_OP_STORE = 0,
    VK_ATTACHMENT_STORE_OP_DONT_CARE = 1,
};

enum VkPipelineBindPoint {
    VK_PIPELINE_BIND_POINT_GRAPHICS = 0,
    VK_PIPELINE_BIND_POINT_COMPUTE = 1,
};

enum VkPrimitiveTopology {
    VK_PRIMITIVE_TOPOLOGY_POINT_LIST = 0,
    VK_PRIMITIVE_TOPOLOGY_LINE_LIST = 1,
    VK_PRIMITIVE_TOPOLOGY_LINE_STRIP = 2,
    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST = 3,
    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP = 4,
};

enum VkPolygonMode {
    VK_POLYGON_MODE_FILL = 0,
    VK_POLYGON_MODE_LINE = 1,
    VK_POLYGON_MODE_POINT = 2,
};

enum VkCullModeFlagBits {
    VK_CULL_MODE_NONE = 0,
    VK_CULL_MODE_FRONT_BIT = 0x00000001,
    VK_CULL_MODE_BACK_BIT = 0x00000002,
    VK_CULL_MODE_FRONT_AND_BACK = 0x00000003,
};

enum VkFrontFace {
    VK_FRONT_FACE_COUNTER_CLOCKWISE = 0,
    VK_FRONT_FACE_CLOCKWISE = 1,
};

enum VkCompareOp {
    VK_COMPARE_OP_NEVER = 0,
    VK_COMPARE_OP_LESS = 1,
    VK_COMPARE_OP_EQUAL = 2,
    VK_COMPARE_OP_LESS_OR_EQUAL = 3,
    VK_COMPARE_OP_GREATER = 4,
    VK_COMPARE_OP_NOT_EQUAL = 5,
    VK_COMPARE_OP_GREATER_OR_EQUAL = 6,
    VK_COMPARE_OP_ALWAYS = 7,
};

enum VkDynamicState {
    VK_DYNAMIC_STATE_VIEWPORT = 0,
    VK_DYNAMIC_STATE_SCISSOR = 1,
};

enum VkShaderStageFlagBits {
    VK_SHADER_STAGE_VERTEX_BIT = 0x00000001,
    VK_SHADER_STAGE_FRAGMENT_BIT = 0x00000010,
    VK_SHADER_STAGE_ALL_GRAPHICS = 0x0000001F,
};

enum VkBufferUsageFlagBits {
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT = 0x00000001,
    VK_BUFFER_USAGE_TRANSFER_DST_BIT = 0x00000002,
    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT = 0x00000010,
    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT = 0x00000020,
    VK_BUFFER_USAGE_INDEX_BUFFER_BIT = 0x00000040,
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT = 0x00000080,
};

enum VkMemoryPropertyFlagBits {
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT = 0x00000001,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT = 0x00000002,
    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT = 0x00000004,
};

enum VkQueueFlagBits {
    VK_QUEUE_GRAPHICS_BIT = 0x00000001,
    VK_QUEUE_COMPUTE_BIT = 0x00000002,
    VK_QUEUE_TRANSFER_BIT = 0x00000004,
};

enum VkImageUsageFlagBits {
    VK_IMAGE_USAGE_TRANSFER_SRC_BIT = 0x00000001,
    VK_IMAGE_USAGE_TRANSFER_DST_BIT = 0x00000002,
    VK_IMAGE_USAGE_SAMPLED_BIT = 0x00000004,
    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT = 0x00000010,
    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT = 0x00000020,
};

enum VkImageAspectFlagBits {
    VK_IMAGE_ASPECT_COLOR_BIT = 0x00000001,
    VK_IMAGE_ASPECT_DEPTH_BIT = 0x00000002,
};

enum VkFilter {
    VK_FILTER_NEAREST = 0,
    VK_FILTER_LINEAR = 1,
};

enum VkSamplerMipmapMode {
    VK_SAMPLER_MIPMAP_MODE_NEAREST = 0,
    VK_SAMPLER_MIPMAP_MODE_LINEAR = 1,
};

enum VkSamplerAddressMode {
    VK_SAMPLER_ADDRESS_MODE_REPEAT = 0,
    VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT = 1,
    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE = 2,
};

enum VkVertexInputRate {
    VK_VERTEX_INPUT_RATE_VERTEX = 0,
    VK_VERTEX_INPUT_RATE_INSTANCE = 1,
};

// Opaque handles
#define VK_DEFINE_HANDLE(object) typedef struct object##_T* object;
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef uint64_t object;

VK_DEFINE_HANDLE(VkInstance)
VK_DEFINE_HANDLE(VkPhysicalDevice)
VK_DEFINE_HANDLE(VkDevice)
VK_DEFINE_HANDLE(VkQueue)
VK_DEFINE_HANDLE(VkCommandBuffer)

VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSurfaceKHR)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSwapchainKHR)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkImage)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkImageView)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkBuffer)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkDeviceMemory)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkShaderModule)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkPipelineLayout)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkRenderPass)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkPipeline)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkFramebuffer)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkCommandPool)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSampler)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSemaphore)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkFence)

struct VkExtent2D {
    uint32_t width;
    uint32_t height;
};

struct VkExtent3D {
    uint32_t width;
    uint32_t height;
    uint32_t depth;
};

struct VkOffset2D {
    int32_t x;
    int32_t y;
};

struct VkOffset3D {
    int32_t x;
    int32_t y;
    int32_t z;
};

struct VkRect2D {
    VkOffset2D offset;
    VkExtent2D extent;
};

struct VkViewport {
    float x;
    float y;
    float width;
    float height;
    float minDepth;
    float maxDepth;
};

struct VkClearColorValue {
    float float32[4];
};

struct VkClearDepthStencilValue {
    float depth;
    uint32_t stencil;
};

union VkClearValue {
    VkClearColorValue color;
    VkClearDepthStencilValue depthStencil;
};

struct VkApplicationInfo {
    VkStructureType sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    const void* pNext = nullptr;
    const char* pApplicationName = nullptr;
    uint32_t applicationVersion = 0;
    const char* pEngineName = nullptr;
    uint32_t engineVersion = 0;
    uint32_t apiVersion = 0;
};

struct VkInstanceCreateInfo {
    VkStructureType sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    const void* pNext = nullptr;
    VkFlags flags = 0;
    const VkApplicationInfo* pApplicationInfo = nullptr;
    uint32_t enabledLayerCount = 0;
    const char* const* ppEnabledLayerNames = nullptr;
    uint32_t enabledExtensionCount = 0;
    const char* const* ppEnabledExtensionNames = nullptr;
};

struct VkWin32SurfaceCreateInfoKHR {
    VkStructureType sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    const void* pNext = nullptr;
    VkFlags flags = 0;
    HINSTANCE hinstance = nullptr;
    HWND hwnd = nullptr;
};

struct VkQueueFamilyProperties {
    VkFlags queueFlags;
    uint32_t queueCount;
    uint32_t timestampValidBits;
    VkExtent3D minImageTransferGranularity;
};

struct VkDeviceQueueCreateInfo {
    VkStructureType sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    const void* pNext = nullptr;
    VkFlags flags = 0;
    uint32_t queueFamilyIndex = 0;
    uint32_t queueCount = 0;
    const float* pQueuePriorities = nullptr;
};

struct VkDeviceCreateInfo {
    VkStructureType sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    const void* pNext = nullptr;
    VkFlags flags = 0;
    uint32_t queueCreateInfoCount = 0;
    const VkDeviceQueueCreateInfo* pQueueCreateInfos = nullptr;
    uint32_t enabledLayerCount = 0;
    const char* const* ppEnabledLayerNames = nullptr;
    uint32_t enabledExtensionCount = 0;
    const char* const* ppEnabledExtensionNames = nullptr;
    const void* pEnabledFeatures = nullptr;
};

struct VkSurfaceCapabilitiesKHR {
    uint32_t minImageCount;
    uint32_t maxImageCount;
    VkExtent2D currentExtent;
    VkExtent2D minImageExtent;
    VkExtent2D maxImageExtent;
    uint32_t maxImageArrayLayers;
    VkFlags supportedTransforms;
    VkFlags currentTransform;
    VkFlags supportedCompositeAlpha;
    VkFlags supportedUsageFlags;
};

struct VkSurfaceFormatKHR {
    VkFormat format;
    VkColorSpaceKHR colorSpace;
};

struct VkSwapchainCreateInfoKHR {
    VkStructureType sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    const void* pNext = nullptr;
    VkFlags flags = 0;
    VkSurfaceKHR surface = 0;
    uint32_t minImageCount = 0;
    VkFormat imageFormat = VK_FORMAT_UNDEFINED;
    VkColorSpaceKHR imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    VkExtent2D imageExtent = {0, 0};
    uint32_t imageArrayLayers = 0;
    VkFlags imageUsage = 0;
    VkSharingMode imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    uint32_t queueFamilyIndexCount = 0;
    const uint32_t* pQueueFamilyIndices = nullptr;
    VkFlags preTransform = 0;
    VkFlags compositeAlpha = 0;
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    VkBool32 clipped = VK_TRUE;
    VkSwapchainKHR oldSwapchain = 0;
};

struct VkImageCreateInfo {
    VkStructureType sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    const void* pNext = nullptr;
    VkFlags flags = 0;
    uint32_t imageType = 1; // VK_IMAGE_TYPE_2D
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkExtent3D extent = {0, 0, 1};
    uint32_t mipLevels = 1;
    uint32_t arrayLayers = 1;
    uint32_t samples = 1;
    uint32_t tiling = 0;
    VkFlags usage = 0;
    VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    uint32_t queueFamilyIndexCount = 0;
    const uint32_t* pQueueFamilyIndices = nullptr;
    VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
};

struct VkComponentMapping {
    uint32_t r = 0, g = 0, b = 0, a = 0;
};

struct VkImageSubresourceRange {
    VkFlags aspectMask = 0;
    uint32_t baseMipLevel = 0;
    uint32_t levelCount = 1;
    uint32_t baseArrayLayer = 0;
    uint32_t layerCount = 1;
};

struct VkImageViewCreateInfo {
    VkStructureType sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    const void* pNext = nullptr;
    VkFlags flags = 0;
    VkImage image = 0;
    uint32_t viewType = 1; // VK_IMAGE_VIEW_TYPE_2D
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkComponentMapping components = {};
    VkImageSubresourceRange subresourceRange = {};
};

struct VkAttachmentDescription {
    VkFlags flags = 0;
    VkFormat format = VK_FORMAT_UNDEFINED;
    uint32_t samples = 1; // 1 sample
    VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageLayout finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
};

struct VkAttachmentReference {
    uint32_t attachment = 0;
    VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
};

struct VkSubpassDescription {
    VkFlags flags = 0;
    VkPipelineBindPoint pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    uint32_t inputAttachmentCount = 0;
    const VkAttachmentReference* pInputAttachments = nullptr;
    uint32_t colorAttachmentCount = 0;
    const VkAttachmentReference* pColorAttachments = nullptr;
    const VkAttachmentReference* pResolveAttachments = nullptr;
    const VkAttachmentReference* pDepthStencilAttachment = nullptr;
    uint32_t preserveAttachmentCount = 0;
    const uint32_t* pPreserveAttachments = nullptr;
};

struct VkRenderPassCreateInfo {
    VkStructureType sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    const void* pNext = nullptr;
    VkFlags flags = 0;
    uint32_t attachmentCount = 0;
    const VkAttachmentDescription* pAttachments = nullptr;
    uint32_t subpassCount = 0;
    const VkSubpassDescription* pSubpasses = nullptr;
    uint32_t dependencyCount = 0;
    const void* pDependencies = nullptr;
};

struct VkFramebufferCreateInfo {
    VkStructureType sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    const void* pNext = nullptr;
    VkFlags flags = 0;
    VkRenderPass renderPass = 0;
    uint32_t attachmentCount = 0;
    const VkImageView* pAttachments = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t layers = 1;
};

struct VkBufferCreateInfo {
    VkStructureType sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    const void* pNext = nullptr;
    VkFlags flags = 0;
    VkDeviceSize size = 0;
    VkFlags usage = 0;
    VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    uint32_t queueFamilyIndexCount = 0;
    const uint32_t* pQueueFamilyIndices = nullptr;
};

struct VkMemoryRequirements {
    VkDeviceSize size;
    VkDeviceSize alignment;
    uint32_t memoryTypeBits;
};

struct VkMemoryAllocateInfo {
    VkStructureType sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    const void* pNext = nullptr;
    VkDeviceSize allocationSize = 0;
    uint32_t memoryTypeIndex = 0;
};

struct VkPhysicalDeviceMemoryProperties {
    uint32_t memoryTypeCount;
    struct {
        VkFlags propertyFlags;
        uint32_t heapIndex;
    } memoryTypes[32];
    uint32_t memoryHeapCount;
    struct {
        VkDeviceSize size;
        VkFlags flags;
    } memoryHeaps[16];
};

struct VkPushConstantRange {
    VkFlags stageFlags = 0;
    uint32_t offset = 0;
    uint32_t size = 0;
};

struct VkPipelineLayoutCreateInfo {
    VkStructureType sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    const void* pNext = nullptr;
    VkFlags flags = 0;
    uint32_t setLayoutCount = 0;
    const void* pSetLayouts = nullptr;
    uint32_t pushConstantRangeCount = 0;
    const VkPushConstantRange* pPushConstantRanges = nullptr;
};

struct VkShaderModuleCreateInfo {
    VkStructureType sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    const void* pNext = nullptr;
    VkFlags flags = 0;
    size_t codeSize = 0;
    const uint32_t* pCode = nullptr;
};

struct VkPipelineShaderStageCreateInfo {
    VkStructureType sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    const void* pNext = nullptr;
    VkFlags flags = 0;
    VkShaderStageFlagBits stage = VK_SHADER_STAGE_VERTEX_BIT;
    VkShaderModule module = 0;
    const char* pName = "main";
    const void* pSpecializationInfo = nullptr;
};

struct VkVertexInputBindingDescription {
    uint32_t binding;
    uint32_t stride;
    VkVertexInputRate inputRate;
};

struct VkVertexInputAttributeDescription {
    uint32_t location;
    uint32_t binding;
    VkFormat format;
    uint32_t offset;
};

struct VkPipelineVertexInputStateCreateInfo {
    VkStructureType sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    const void* pNext = nullptr;
    VkFlags flags = 0;
    uint32_t vertexBindingDescriptionCount = 0;
    const VkVertexInputBindingDescription* pVertexBindingDescriptions = nullptr;
    uint32_t vertexAttributeDescriptionCount = 0;
    const VkVertexInputAttributeDescription* pVertexAttributeDescriptions = nullptr;
};

struct VkPipelineInputAssemblyStateCreateInfo {
    VkStructureType sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    const void* pNext = nullptr;
    VkFlags flags = 0;
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkBool32 primitiveRestartEnable = VK_FALSE;
};

struct VkPipelineViewportStateCreateInfo {
    VkStructureType sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    const void* pNext = nullptr;
    VkFlags flags = 0;
    uint32_t viewportCount = 1;
    const VkViewport* pViewports = nullptr;
    uint32_t scissorCount = 1;
    const VkRect2D* pScissors = nullptr;
};

struct VkPipelineRasterizationStateCreateInfo {
    VkStructureType sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    const void* pNext = nullptr;
    VkFlags flags = 0;
    VkBool32 depthClampEnable = VK_FALSE;
    VkBool32 rasterizerDiscardEnable = VK_FALSE;
    VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
    VkFlags cullMode = VK_CULL_MODE_BACK_BIT;
    VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    VkBool32 depthBiasEnable = VK_FALSE;
    float depthBiasConstantFactor = 0.0f;
    float depthBiasClamp = 0.0f;
    float depthBiasSlopeFactor = 0.0f;
    float lineWidth = 1.0f;
};

struct VkPipelineMultisampleStateCreateInfo {
    VkStructureType sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    const void* pNext = nullptr;
    VkFlags flags = 0;
    uint32_t rasterizationSamples = 1;
    VkBool32 sampleShadingEnable = VK_FALSE;
    float minSampleShading = 1.0f;
    const uint32_t* pSampleMask = nullptr;
    VkBool32 alphaToCoverageEnable = VK_FALSE;
    VkBool32 alphaToOneEnable = VK_FALSE;
};

struct VkStencilOpState {
    uint32_t failOp = 0, passOp = 0, depthFailOp = 0, compareOp = 0, compareMask = 0, writeMask = 0, reference = 0;
};

struct VkPipelineDepthStencilStateCreateInfo {
    VkStructureType sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    const void* pNext = nullptr;
    VkFlags flags = 0;
    VkBool32 depthTestEnable = VK_TRUE;
    VkBool32 depthWriteEnable = VK_TRUE;
    VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    VkBool32 depthBoundsTestEnable = VK_FALSE;
    VkBool32 stencilTestEnable = VK_FALSE;
    VkStencilOpState front = {};
    VkStencilOpState back = {};
    float minDepthBounds = 0.0f;
    float maxDepthBounds = 1.0f;
};

struct VkPipelineColorBlendAttachmentState {
    VkBool32 blendEnable = VK_FALSE;
    uint32_t srcColorBlendFactor = 1; // ONE
    uint32_t dstColorBlendFactor = 0; // ZERO
    uint32_t colorBlendOp = 0; // ADD
    uint32_t srcAlphaBlendFactor = 1;
    uint32_t dstAlphaBlendFactor = 0;
    uint32_t alphaBlendOp = 0;
    VkFlags colorWriteMask = 0xF; // RGBA
};

struct VkPipelineColorBlendStateCreateInfo {
    VkStructureType sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    const void* pNext = nullptr;
    VkFlags flags = 0;
    VkBool32 logicOpEnable = VK_FALSE;
    uint32_t logicOp = 0;
    uint32_t attachmentCount = 1;
    const VkPipelineColorBlendAttachmentState* pAttachments = nullptr;
    float blendConstants[4] = {0, 0, 0, 0};
};

struct VkPipelineDynamicStateCreateInfo {
    VkStructureType sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    const void* pNext = nullptr;
    VkFlags flags = 0;
    uint32_t dynamicStateCount = 0;
    const VkDynamicState* pDynamicStates = nullptr;
};

struct VkGraphicsPipelineCreateInfo {
    VkStructureType sType = (VkStructureType)34; // VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO
    const void* pNext = nullptr;
    VkFlags flags = 0;
    uint32_t stageCount = 0;
    const VkPipelineShaderStageCreateInfo* pStages = nullptr;
    const VkPipelineVertexInputStateCreateInfo* pVertexInputState = nullptr;
    const VkPipelineInputAssemblyStateCreateInfo* pInputAssemblyState = nullptr;
    const void* pTessellationState = nullptr;
    const VkPipelineViewportStateCreateInfo* pViewportState = nullptr;
    const VkPipelineRasterizationStateCreateInfo* pRasterizationState = nullptr;
    const VkPipelineMultisampleStateCreateInfo* pMultisampleState = nullptr;
    const VkPipelineDepthStencilStateCreateInfo* pDepthStencilState = nullptr;
    const VkPipelineColorBlendStateCreateInfo* pColorBlendState = nullptr;
    const VkPipelineDynamicStateCreateInfo* pDynamicState = nullptr;
    VkPipelineLayout layout = 0;
    VkRenderPass renderPass = 0;
    uint32_t subpass = 0;
    VkPipeline basePipelineHandle = 0;
    int32_t basePipelineIndex = -1;
};

struct VkCommandPoolCreateInfo {
    VkStructureType sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    const void* pNext = nullptr;
    VkFlags flags = 2; // VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
    uint32_t queueFamilyIndex = 0;
};

struct VkCommandBufferAllocateInfo {
    VkStructureType sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    const void* pNext = nullptr;
    VkCommandPool commandPool = 0;
    uint32_t level = 0; // VK_COMMAND_BUFFER_LEVEL_PRIMARY
    uint32_t commandBufferCount = 0;
};

struct VkCommandBufferBeginInfo {
    VkStructureType sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    const void* pNext = nullptr;
    VkFlags flags = 0;
    const void* pInheritanceInfo = nullptr;
};

struct VkRenderPassBeginInfo {
    VkStructureType sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    const void* pNext = nullptr;
    VkRenderPass renderPass = 0;
    VkFramebuffer framebuffer = 0;
    VkRect2D renderArea = {};
    uint32_t clearValueCount = 0;
    const VkClearValue* pClearValues = nullptr;
};

struct VkSemaphoreCreateInfo {
    VkStructureType sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    const void* pNext = nullptr;
    VkFlags flags = 0;
};

struct VkFenceCreateInfo {
    VkStructureType sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    const void* pNext = nullptr;
    VkFlags flags = 1; // VK_FENCE_CREATE_SIGNALED_BIT
};

struct VkSubmitInfo {
    VkStructureType sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    const void* pNext = nullptr;
    uint32_t waitSemaphoreCount = 0;
    const VkSemaphore* pWaitSemaphores = nullptr;
    const VkFlags* pWaitDstStageMask = nullptr;
    uint32_t commandBufferCount = 0;
    const VkCommandBuffer* pCommandBuffers = nullptr;
    uint32_t signalSemaphoreCount = 0;
    const VkSemaphore* pSignalSemaphores = nullptr;
};

struct VkPresentInfoKHR {
    VkStructureType sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    const void* pNext = nullptr;
    uint32_t waitSemaphoreCount = 0;
    const VkSemaphore* pWaitSemaphores = nullptr;
    uint32_t swapchainCount = 0;
    const VkSwapchainKHR* pSwapchains = nullptr;
    const uint32_t* pImageIndices = nullptr;
    VkResult* pResults = nullptr;
};

struct VkBufferCopy {
    VkDeviceSize srcOffset;
    VkDeviceSize dstOffset;
    VkDeviceSize size;
};

typedef VkFlags VkPipelineStageFlags;

// Function Pointers
typedef void* (VKAPI_PTR *PFN_vkVoidFunction)(void);
typedef PFN_vkVoidFunction (VKAPI_PTR *PFN_vkGetInstanceProcAddr)(VkInstance instance, const char* pName);
typedef PFN_vkVoidFunction (VKAPI_PTR *PFN_vkGetDeviceProcAddr)(VkDevice device, const char* pName);

typedef VkResult (VKAPI_PTR *PFN_vkCreateInstance)(const VkInstanceCreateInfo* pCreateInfo, const void* pAllocator, VkInstance* pInstance);
typedef void (VKAPI_PTR *PFN_vkDestroyInstance)(VkInstance instance, const void* pAllocator);
typedef VkResult (VKAPI_PTR *PFN_vkEnumeratePhysicalDevices)(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices);
typedef void (VKAPI_PTR *PFN_vkGetPhysicalDeviceQueueFamilyProperties)(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties);
typedef void (VKAPI_PTR *PFN_vkGetPhysicalDeviceMemoryProperties)(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties);
typedef VkResult (VKAPI_PTR *PFN_vkCreateDevice)(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const void* pAllocator, VkDevice* pDevice);
typedef void (VKAPI_PTR *PFN_vkDestroyDevice)(VkDevice device, const void* pAllocator);
typedef void (VKAPI_PTR *PFN_vkGetDeviceQueue)(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue);
typedef VkResult (VKAPI_PTR *PFN_vkDeviceWaitIdle)(VkDevice device);

typedef VkResult (VKAPI_PTR *PFN_vkCreateWin32SurfaceKHR)(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo, const void* pAllocator, VkSurfaceKHR* pSurface);
typedef void (VKAPI_PTR *PFN_vkDestroySurfaceKHR)(VkInstance instance, VkSurfaceKHR surface, const void* pAllocator);
typedef VkResult (VKAPI_PTR *PFN_vkGetPhysicalDeviceSurfaceSupportKHR)(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported);
typedef VkResult (VKAPI_PTR *PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities);
typedef VkResult (VKAPI_PTR *PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats);
typedef VkResult (VKAPI_PTR *PFN_vkCreateSwapchainKHR)(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const void* pAllocator, VkSwapchainKHR* pSwapchain);
typedef void (VKAPI_PTR *PFN_vkDestroySwapchainKHR)(VkDevice device, VkSwapchainKHR swapchain, const void* pAllocator);
typedef VkResult (VKAPI_PTR *PFN_vkGetSwapchainImagesKHR)(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages);
typedef VkResult (VKAPI_PTR *PFN_vkAcquireNextImageKHR)(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex);
typedef VkResult (VKAPI_PTR *PFN_vkQueuePresentKHR)(VkQueue queue, const VkPresentInfoKHR* pPresentInfo);

typedef VkResult (VKAPI_PTR *PFN_vkCreateBuffer)(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const void* pAllocator, VkBuffer* pBuffer);
typedef void (VKAPI_PTR *PFN_vkDestroyBuffer)(VkDevice device, VkBuffer buffer, const void* pAllocator);
typedef void (VKAPI_PTR *PFN_vkGetBufferMemoryRequirements)(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements);
typedef VkResult (VKAPI_PTR *PFN_vkAllocateMemory)(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const void* pAllocator, VkDeviceMemory* pMemory);
typedef void (VKAPI_PTR *PFN_vkFreeMemory)(VkDevice device, VkDeviceMemory memory, const void* pAllocator);
typedef VkResult (VKAPI_PTR *PFN_vkBindBufferMemory)(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset);
typedef VkResult (VKAPI_PTR *PFN_vkMapMemory)(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkFlags flags, void** ppData);
typedef void (VKAPI_PTR *PFN_vkUnmapMemory)(VkDevice device, VkDeviceMemory memory);
typedef VkResult (VKAPI_PTR *PFN_vkCreateImage)(VkDevice device, const VkImageCreateInfo* pCreateInfo, const void* pAllocator, VkImage* pImage);
typedef void (VKAPI_PTR *PFN_vkDestroyImage)(VkDevice device, VkImage image, const void* pAllocator);
typedef void (VKAPI_PTR *PFN_vkGetImageMemoryRequirements)(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements);
typedef VkResult (VKAPI_PTR *PFN_vkBindImageMemory)(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset);
typedef VkResult (VKAPI_PTR *PFN_vkCreateImageView)(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, const void* pAllocator, VkImageView* pView);
typedef void (VKAPI_PTR *PFN_vkDestroyImageView)(VkDevice device, VkImageView imageView, const void* pAllocator);

typedef VkResult (VKAPI_PTR *PFN_vkCreateShaderModule)(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const void* pAllocator, VkShaderModule* pShaderModule);
typedef void (VKAPI_PTR *PFN_vkDestroyShaderModule)(VkDevice device, VkShaderModule shaderModule, const void* pAllocator);
typedef VkResult (VKAPI_PTR *PFN_vkCreatePipelineLayout)(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const void* pAllocator, VkPipelineLayout* pPipelineLayout);
typedef void (VKAPI_PTR *PFN_vkDestroyPipelineLayout)(VkDevice device, VkPipelineLayout pipelineLayout, const void* pAllocator);
typedef VkResult (VKAPI_PTR *PFN_vkCreateRenderPass)(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const void* pAllocator, VkRenderPass* pRenderPass);
typedef void (VKAPI_PTR *PFN_vkDestroyRenderPass)(VkDevice device, VkRenderPass renderPass, const void* pAllocator);
typedef VkResult (VKAPI_PTR *PFN_vkCreateGraphicsPipelines)(VkDevice device, uint64_t pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const void* pAllocator, VkPipeline* pPipelines);
typedef void (VKAPI_PTR *PFN_vkDestroyPipeline)(VkDevice device, VkPipeline pipeline, const void* pAllocator);
typedef VkResult (VKAPI_PTR *PFN_vkCreateFramebuffer)(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo, const void* pAllocator, VkFramebuffer* pFramebuffer);
typedef void (VKAPI_PTR *PFN_vkDestroyFramebuffer)(VkDevice device, VkFramebuffer framebuffer, const void* pAllocator);

typedef VkResult (VKAPI_PTR *PFN_vkCreateCommandPool)(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, const void* pAllocator, VkCommandPool* pCommandPool);
typedef void (VKAPI_PTR *PFN_vkDestroyCommandPool)(VkDevice device, VkCommandPool commandPool, const void* pAllocator);
typedef VkResult (VKAPI_PTR *PFN_vkAllocateCommandBuffers)(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers);
typedef void (VKAPI_PTR *PFN_vkFreeCommandBuffers)(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers);
typedef VkResult (VKAPI_PTR *PFN_vkBeginCommandBuffer)(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo);
typedef VkResult (VKAPI_PTR *PFN_vkEndCommandBuffer)(VkCommandBuffer commandBuffer);
typedef VkResult (VKAPI_PTR *PFN_vkResetCommandBuffer)(VkCommandBuffer commandBuffer, VkFlags flags);
typedef void (VKAPI_PTR *PFN_vkCmdBeginRenderPass)(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, uint32_t contents);
typedef void (VKAPI_PTR *PFN_vkCmdEndRenderPass)(VkCommandBuffer commandBuffer);
typedef void (VKAPI_PTR *PFN_vkCmdBindPipeline)(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);
typedef void (VKAPI_PTR *PFN_vkCmdSetViewport)(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports);
typedef void (VKAPI_PTR *PFN_vkCmdSetScissor)(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors);
typedef void (VKAPI_PTR *PFN_vkCmdBindVertexBuffers)(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets);
typedef void (VKAPI_PTR *PFN_vkCmdBindIndexBuffer)(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t indexType);
typedef void (VKAPI_PTR *PFN_vkCmdPushConstants)(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues);
typedef void (VKAPI_PTR *PFN_vkCmdDraw)(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
typedef void (VKAPI_PTR *PFN_vkCmdDrawIndexed)(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
typedef void (VKAPI_PTR *PFN_vkCmdCopyBuffer)(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions);

typedef VkResult (VKAPI_PTR *PFN_vkCreateSemaphore)(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, const void* pAllocator, VkSemaphore* pSemaphore);
typedef void (VKAPI_PTR *PFN_vkDestroySemaphore)(VkDevice device, VkSemaphore semaphore, const void* pAllocator);
typedef VkResult (VKAPI_PTR *PFN_vkCreateFence)(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const void* pAllocator, VkFence* pFence);
typedef void (VKAPI_PTR *PFN_vkDestroyFence)(VkDevice device, VkFence fence, const void* pAllocator);
typedef VkResult (VKAPI_PTR *PFN_vkWaitForFences)(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout);
typedef VkResult (VKAPI_PTR *PFN_vkResetFences)(VkDevice device, uint32_t fenceCount, const VkFence* pFences);
typedef VkResult (VKAPI_PTR *PFN_vkQueueSubmit)(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence);

// Global Function Pointer Declarations
extern PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
extern PFN_vkCreateInstance vkCreateInstance;
extern PFN_vkDestroyInstance vkDestroyInstance;
extern PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
extern PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
extern PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
extern PFN_vkCreateDevice vkCreateDevice;
extern PFN_vkDestroyDevice vkDestroyDevice;
extern PFN_vkGetDeviceQueue vkGetDeviceQueue;
extern PFN_vkDeviceWaitIdle vkDeviceWaitIdle;

// Surface / Swapchain
extern PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;
extern PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
extern PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
extern PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
extern PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
extern PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
extern PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
extern PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
extern PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
extern PFN_vkQueuePresentKHR vkQueuePresentKHR;

// Memory / Buffers / Images
extern PFN_vkCreateBuffer vkCreateBuffer;
extern PFN_vkDestroyBuffer vkDestroyBuffer;
extern PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
extern PFN_vkAllocateMemory vkAllocateMemory;
extern PFN_vkFreeMemory vkFreeMemory;
extern PFN_vkBindBufferMemory vkBindBufferMemory;
extern PFN_vkMapMemory vkMapMemory;
extern PFN_vkUnmapMemory vkUnmapMemory;
extern PFN_vkCreateImage vkCreateImage;
extern PFN_vkDestroyImage vkDestroyImage;
extern PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
extern PFN_vkBindImageMemory vkBindImageMemory;
extern PFN_vkCreateImageView vkCreateImageView;
extern PFN_vkDestroyImageView vkDestroyImageView;

// Pipelines / Shaders / RenderPass
extern PFN_vkCreateShaderModule vkCreateShaderModule;
extern PFN_vkDestroyShaderModule vkDestroyShaderModule;
extern PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
extern PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;
extern PFN_vkCreateRenderPass vkCreateRenderPass;
extern PFN_vkDestroyRenderPass vkDestroyRenderPass;
extern PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
extern PFN_vkDestroyPipeline vkDestroyPipeline;
extern PFN_vkCreateFramebuffer vkCreateFramebuffer;
extern PFN_vkDestroyFramebuffer vkDestroyFramebuffer;

// Commands & Sync
extern PFN_vkCreateCommandPool vkCreateCommandPool;
extern PFN_vkDestroyCommandPool vkDestroyCommandPool;
extern PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
extern PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
extern PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
extern PFN_vkEndCommandBuffer vkEndCommandBuffer;
extern PFN_vkResetCommandBuffer vkResetCommandBuffer;
extern PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
extern PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
extern PFN_vkCmdBindPipeline vkCmdBindPipeline;
extern PFN_vkCmdSetViewport vkCmdSetViewport;
extern PFN_vkCmdSetScissor vkCmdSetScissor;
extern PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers;
extern PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
extern PFN_vkCmdPushConstants vkCmdPushConstants;
extern PFN_vkCmdDraw vkCmdDraw;
extern PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
extern PFN_vkCmdCopyBuffer vkCmdCopyBuffer;
extern PFN_vkCreateSemaphore vkCreateSemaphore;
extern PFN_vkDestroySemaphore vkDestroySemaphore;
extern PFN_vkCreateFence vkCreateFence;
extern PFN_vkDestroyFence vkDestroyFence;
extern PFN_vkWaitForFences vkWaitForFences;
extern PFN_vkResetFences vkResetFences;
extern PFN_vkQueueSubmit vkQueueSubmit;

bool loadVulkanLoader();
bool loadVulkanInstanceFunctions(VkInstance instance);
bool loadVulkanDeviceFunctions(VkDevice device);

} // namespace Aetheria
