#include "DescriptorTable.h"
#include <utils/MatrixBuffer.h>

#include <iostream>

/* --------------------------------- FrameDescriptor --------------------------------- */

void FrameDescriptor::initialize(const std::vector<Material*>& materials)
{
    for (const auto& material : materials)
    {
        m_descriptors[material->materialId()] = DescriptorEntry();
    }
}

void FrameDescriptor::cleanUp(RenderContext& renderContext)
{
    for (auto descriptorIt : m_descriptors) {
        DescriptorEntry& entry = descriptorIt.second;
        vkDestroyBuffer(renderContext.device(), entry.buffer, nullptr);
        vkFreeMemory(renderContext.device(), entry.memory, nullptr);
    }
}

DescriptorEntry& FrameDescriptor::getDescriptorEntry(MaterialID materialId)
{
    return m_descriptors.at(materialId);
}

/* --------------------------------- DescriptorTable --------------------------------- */

DescriptorTable::DescriptorTable(RenderContext& renderContext):
    m_renderContext(renderContext)
{

}

void DescriptorTable::createDescriptorPool()
{
    uint32_t swapChainImageSize = m_renderContext.swapChain().size();
    std::vector<VkDescriptorPoolSize> descriptorPoolSizes;

    VkDescriptorPoolSize globalDescriptor;
    globalDescriptor.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    globalDescriptor.descriptorCount = static_cast<uint32_t>(swapChainImageSize);
    descriptorPoolSizes.push_back(globalDescriptor);
    
    for (auto& material : m_materials) {
        std::vector<VkDescriptorSetLayoutBinding>& descriptorBindings = material->descriptorBindings();

        for (auto& binding : descriptorBindings) {
            VkDescriptorPoolSize descriptor;
            descriptor.type = binding.descriptorType;
            descriptor.descriptorCount = static_cast<uint32_t>(swapChainImageSize);
            descriptorPoolSizes.push_back(descriptor);
        }
    }

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
    poolInfo.pPoolSizes = descriptorPoolSizes.data();
    // 2 sets for each swap chain image
    poolInfo.maxSets = static_cast<uint32_t>(2 * swapChainImageSize);

    if (vkCreateDescriptorPool(m_renderContext.device(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void DescriptorTable::createDescriptorLayouts()
{
    VkDescriptorSetLayoutBinding globalBufferBinding = MatrixBuffer::descriptorBinding();
    std::array<VkDescriptorSetLayoutBinding, 1> globalLayoutBindings = { globalBufferBinding };
    VkDescriptorSetLayoutCreateInfo globalLayoutInfo{};
    globalLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    globalLayoutInfo.bindingCount = 1;
    globalLayoutInfo.pBindings = globalLayoutBindings.data();

    if (vkCreateDescriptorSetLayout(m_renderContext.device(), &globalLayoutInfo, nullptr, &m_globalUniformDescriptorLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create global descriptor set layout!");
    }

    for (auto& material : m_materials)
    {
        material->createDescriptorLayouts(m_renderContext);
    }
}

void DescriptorTable::createDescriptorBuffers()
{
    uint32_t swapChainImageSize = m_renderContext.swapChain().size();
    VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    // Global descriptor
    VkDeviceSize matrixBufferSize = sizeof(MatrixBuffer::BufferData);
    m_globalDescriptorEntries.resize(swapChainImageSize);
    for (auto& descriptorEntry : m_globalDescriptorEntries) {
        m_renderContext.createBuffer(matrixBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, memoryPropertyFlags, descriptorEntry.buffer, descriptorEntry.memory);
    }

    // Material descriptors
    // To be moved in initiliaze
    m_frameDescriptors.resize(swapChainImageSize);

    for (auto& frameDescriptor : m_frameDescriptors) {
        frameDescriptor.initialize(m_materials);
        for (auto& material : m_materials) {
            auto& descriptorEntry = frameDescriptor.getDescriptorEntry(material->materialId());
            material->createDescriptorBuffer(m_renderContext, descriptorEntry.buffer, descriptorEntry.memory);
        }
    }
}

void DescriptorTable::createFrameDescriptors()
{
    uint32_t swapChainImageSize = m_renderContext.swapChain().size();

    /* ------------------------- Create Descriptor ------------------------- */
    for (auto& globalDescriptor : m_globalDescriptorEntries) {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &m_globalUniformDescriptorLayout;
        if (vkAllocateDescriptorSets(m_renderContext.device(), &allocInfo, &globalDescriptor.descriptorSet) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }
    }

    for (auto& frameDescriptor : m_frameDescriptors) {

        for (auto& material : m_materials) {
            DescriptorEntry& descriptorEntry = frameDescriptor.getDescriptorEntry(material->materialId());
            std::vector<VkDescriptorSetLayout> descriptorLayout = { material->descriptorLayout() };
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = m_descriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = descriptorLayout.data();

            if (vkAllocateDescriptorSets(m_renderContext.device(), &allocInfo, &descriptorEntry.descriptorSet) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }
        }
    }

    /* ------------------------- Fill Descriptor ------------------------- */

    std::vector<VkWriteDescriptorSet> descriptorWrites;
    for (size_t i = 0; i < m_globalDescriptorEntries.size(); i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_globalDescriptorEntries.at(i).buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(MatrixBuffer::BufferData);

        VkWriteDescriptorSet descriptorWrite;
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_globalDescriptorEntries.at(i).descriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        descriptorWrites.push_back(descriptorWrite);
    }

    vkUpdateDescriptorSets(m_renderContext.device(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

    for (auto& frameDescriptor : m_frameDescriptors) {
        for (auto& material : m_materials) {
            DescriptorEntry& descriptorEntry = frameDescriptor.getDescriptorEntry(material->materialId());
            material->updateDescriptorSet(m_renderContext, descriptorEntry.descriptorSet, descriptorEntry.buffer);
        }
    }
}

void DescriptorTable::cleanUp()
{
    vkDestroyDescriptorSetLayout(m_renderContext.device(), m_globalUniformDescriptorLayout, nullptr);
    for (auto globalDescriptorIt : m_globalDescriptorEntries) {
        vkDestroyBuffer(m_renderContext.device(), globalDescriptorIt.buffer, nullptr);
        vkFreeMemory(m_renderContext.device(), globalDescriptorIt.memory, nullptr);
    }
    for (auto& frameDescriptor : m_frameDescriptors) {
        frameDescriptor.cleanUp(m_renderContext);
    }
    m_frameDescriptors.clear();

    vkDestroyDescriptorPool(m_renderContext.device(), m_descriptorPool, nullptr);
}

void DescriptorTable::addMaterial(Material* material)
{
    m_materials.push_back(material);
}

FrameDescriptor& DescriptorTable::getFrameDescriptor(size_t frameIndex)
{
    return m_frameDescriptors.at(frameIndex);
}

DescriptorEntry& DescriptorTable::getGlobalDescriptor(size_t frameIndex)
{
    return m_globalDescriptorEntries.at(frameIndex);
}

VkDescriptorSetLayout DescriptorTable::globalDescriptorLayout() const
{
    return m_globalUniformDescriptorLayout;
}