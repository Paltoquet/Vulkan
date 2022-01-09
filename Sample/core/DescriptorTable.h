#pragma once

#include <utils/Material.h> 

#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>
#include <memory>

#include "RenderContext.h"

struct DescriptorEntry {
    VkDescriptorSet descriptorSet;
    VkBuffer buffer;
    VkDeviceMemory memory;
};

class FrameDescriptor
{
public:
    FrameDescriptor() = default;

public:
    void initialize(const std::vector<Material*>& materials);
    void cleanUp(RenderContext& renderContext);
    DescriptorEntry& getDescriptorEntry(MaterialID materialId);

private:
    std::unordered_map<MaterialID, DescriptorEntry> m_descriptors;
};

class DescriptorTable 
{
public:
    DescriptorTable(RenderContext& renderContext);

public:
    void addMaterial(Material* material);

    void createDescriptorPool();
    void createDescriptorLayouts();
    void createDescriptorBuffers();
    void createFrameDescriptors();
    void cleanUp();

    FrameDescriptor& getFrameDescriptor(size_t frameIndex);
    DescriptorEntry& getGlobalDescriptor(size_t frameIndex);
    VkDescriptorSetLayout globalDescriptorLayout() const;

private:
    RenderContext& m_renderContext;
    VkDescriptorPool m_descriptorPool;
    std::vector<FrameDescriptor> m_frameDescriptors;
    std::vector<Material*> m_materials;

    VkDescriptorSetLayout m_globalUniformDescriptorLayout;
    std::vector<DescriptorEntry> m_globalDescriptorEntries;
};
