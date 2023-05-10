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
    
    virtual void createDescriptorLayouts(RenderContext& renderContext);
    virtual void createFrameDescriptorBuffer(RenderContext& renderContext, VkBuffer& buffer, VkDeviceMemory& memory) {}
    virtual void updateFrameDescriptorSet(RenderContext& renderContext, VkDescriptorSet descriptorSet, VkBuffer buffer) {}
    virtual void createMaterialRessources(RenderContext& renderContext) {}
    virtual void updateRessourceDescripotSet(RenderContext& renderContext, VkDescriptorSet descriptorSet) {}

    // change to vector<VkVertexInputAttributeDescription>
    void createPipeline(RenderContext& renderContext, VkRenderPass renderPass, VkVertexInputBindingDescription bindingDescription, 
        std::array<VkVertexInputAttributeDescription, 3> vertexDescription, VkDescriptorSetLayout globalDescriptionLayout);

    virtual void cleanUp(RenderContext& renderContext);
    void destroyPipeline(RenderContext& renderContext);

public:
    MaterialID materialId() const;
    VkPipeline pipeline() const;
    VkPipelineLayout pipelineLayout() const;
    VkDescriptorSetLayout ressourceDescriptorLayout() const;
    VkDescriptorSetLayout frameDescriptorLayout() const;
    // used for gpu ressources, texture, buffers ...
    std::vector<VkDescriptorSetLayoutBinding>& ressourceDescriptorBindings();
    // used for cpu/gpu parameters
    std::vector<VkDescriptorSetLayoutBinding>& frameDescriptorBindings();

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

    VkDescriptorSetLayout m_ressourceDescriptorSetLayout;
    VkDescriptorSetLayout m_frameDescriptorSetLayout;
    std::vector<VkDescriptorSetLayoutBinding> m_ressourceDescriptorBindings;
    std::vector<VkDescriptorSetLayoutBinding> m_frameDescriptorBindings;
    //std::vector<VkDescriptorSetLayoutBindingFlagsCreateInfo> m_bindingStrategies;
};