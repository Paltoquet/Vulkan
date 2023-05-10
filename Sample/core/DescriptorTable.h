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
    DescriptorEntry& getGlobalDescriptorEntry();

private:
    std::unordered_map<MaterialID, DescriptorEntry> m_descriptors;
    DescriptorEntry m_globalDescriptorEntry;
};

class DescriptorTable 
{
public:
    DescriptorTable(RenderContext& renderContext);

public:
    void addMaterial(Material* material);

    void createDescriptorPool();
    void createDescriptorLayouts();
    void createDescriptorRessources();
    void createDescriptorSets();
    void updateDescriptorSets();
    void cleanUp();

    FrameDescriptor& getFrameDescriptor(size_t frameIndex);
    DescriptorEntry& getGlobalDescriptor(size_t frameIndex);
    VkDescriptorSetLayout worldDescriptorLayout() const;

    std::vector<DescriptorEntry> getMaterialFrameDescriptor(MaterialID materialId);
    VkDescriptorSet getMaterialRessourceDescriptor(MaterialID materialId);

private:
    RenderContext& m_renderContext;
    VkDescriptorPool m_descriptorPool;
    VkDescriptorSetLayout m_worldDescriptorLayout;
    std::vector<FrameDescriptor> m_frameDescriptors;
    std::vector<Material*> m_materials;
    std::unordered_map<MaterialID, VkDescriptorSet> m_ressourceDescriptors;
};
