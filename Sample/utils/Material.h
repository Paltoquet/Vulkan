#pragma once

#include <core/RenderContext.h>
#include <array>
#include <vector>
#include <atomic>   
#include <mutex>

using MaterialID = std::size_t;

class Material
{
public:
    Material(VkDevice device, VkShaderModule vertexShader, VkShaderModule fragmentShader);
    virtual ~Material();
    
    void createDescriptorLayouts(RenderContext& renderContext);
    virtual void createDescriptorBuffer(RenderContext& renderContext, VkBuffer& buffer, VkDeviceMemory& memory) = 0;
    virtual void updateDescriptorSet(RenderContext& renderContext, VkDescriptorSet descriptorSet, VkBuffer buffer) = 0;

    // change to vector<VkVertexInputAttributeDescription>
    void createPipeline(RenderContext& renderContext, VkRenderPass renderPass, VkVertexInputBindingDescription bindingDescription, 
        std::array<VkVertexInputAttributeDescription, 3> vertexDescription, VkDescriptorSetLayout globalDescriptionLayout);

    virtual void cleanUp(RenderContext& renderContext);
    void destroyPipeline(RenderContext& renderContext);

public:
    MaterialID materialId() const;
    VkPipeline pipeline() const;
    VkPipelineLayout pipelineLayout() const;
    VkDescriptorSetLayout descriptorLayout() const;
    std::vector<VkDescriptorSetLayoutBinding>& descriptorBindings();

public:
    static std::mutex materialIndexLock;
    static std::atomic<MaterialID> materialCounter;

protected:
    MaterialID m_materialId;
    VkDevice m_device;
    VkPipeline m_pipeline;
    VkPipelineLayout m_pipelineLayout;

    VkShaderModule m_vertexShader;
    VkShaderModule m_fragmentShader;

    VkDescriptorSetLayout m_descriptorSetLayout;
    std::vector<VkDescriptorSetLayoutBinding> m_descriptorBindings;
    std::vector<VkDescriptorSetLayoutBindingFlagsCreateInfo> m_bindingStrategies;
    std::vector<VkWriteDescriptorSet> m_descriptorWrites;
};