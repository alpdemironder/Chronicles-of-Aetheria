#pragma once

#include "VulkanHeaders.hpp"
#include <string>
#include <vector>

namespace Aetheria {

class VulkanContext;

struct VoxelPushConstants {
    float viewProj[16];
    float sunDir_Time[4];
    float fogColor_Density[4];
    float camPos[4];
};

struct HologramPushConstants {
    float viewProj[16];
    float model[16];
    float holoColor[4];
};

struct UIPushConstants {
    float orthoProj[16];
};

class VulkanPipeline {
public:
    VulkanPipeline(VulkanContext* context);
    ~VulkanPipeline();

    bool init(VkRenderPass renderPass);

    VkPipeline getVoxelPipeline() const { return voxelPipeline; }
    VkPipelineLayout getVoxelPipelineLayout() const { return voxelPipelineLayout; }

    VkPipeline getHologramPipeline() const { return hologramPipeline; }
    VkPipelineLayout getHologramPipelineLayout() const { return hologramPipelineLayout; }

    VkPipeline getUIPipeline() const { return uiPipeline; }
    VkPipelineLayout getUIPipelineLayout() const { return uiPipelineLayout; }

private:
    VkShaderModule createShaderModule(const std::string& filepath);
    bool createVoxel(VkRenderPass renderPass);
    bool createHologram(VkRenderPass renderPass);
    bool createUI(VkRenderPass renderPass);

    VulkanContext* context = nullptr;

    VkPipeline voxelPipeline = 0;
    VkPipelineLayout voxelPipelineLayout = 0;

    VkPipeline hologramPipeline = 0;
    VkPipelineLayout hologramPipelineLayout = 0;

    VkPipeline uiPipeline = 0;
    VkPipelineLayout uiPipelineLayout = 0;
};

} // namespace Aetheria
