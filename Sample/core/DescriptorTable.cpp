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
    vkDestroyBuffer(renderContext.device(), m_globalDescriptorEntry.buffer, nullptr);
    vkFreeMemory(renderContext.device(), m_globalDescriptorEntry.memory, nullptr);
}

DescriptorEntry& FrameDescriptor::getDescriptorEntry(MaterialID materialId)
{
    return m_descriptors.at(materialId);
}

DescriptorEntry& FrameDescriptor::getGlobalDescriptorEntry()
{
    return m_globalDescriptorEntry;
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
        std::vector<VkDescriptorSetLayoutBinding>& frameDescriptorBindings = material->frameDescriptorBindings();
        std::vector<VkDescriptorSetLayoutBinding>& ressourceDescriptorBindings = material->ressourceDescriptorBindings();

        // cpu/gpu ressources
        for (auto& binding : frameDescriptorBindings) {
            VkDescriptorPoolSize descriptor;
            descriptor.type = binding.descriptorType;
            descriptor.descriptorCount = static_cast<uint32_t>(swapChainImageSize);
            descriptorPoolSizes.push_back(descriptor);
        }

        // gpu only ressources
        for (auto& binding : ressourceDescriptorBindings) {
            VkDescriptorPoolSize descriptor;
            descriptor.type = binding.descriptorType;
            descriptor.descriptorCount = static_cast<uint32_t>(1);
            descriptorPoolSizes.push_back(descriptor);
        }
    }

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
    poolInfo.pPoolSizes = descriptorPoolSizes.data();
    // (nb materials + globalDescriptor) * nbSwapChainImages
    uint32_t nb_descriptorPerMaterial = 2;
    poolInfo.maxSets = static_cast<uint32_t>((m_materials.size() + 1) * swapChainImageSize * nb_descriptorPerMaterial);

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

    if (vkCreateDescriptorSetLayout(m_renderContext.device(), &globalLayoutInfo, nullptr, &m_worldDescriptorLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create global descriptor set layout!");
    }

    for (auto& material : m_materials)
    {
        material->createDescriptorLayouts(m_renderContext);
    }
}

void DescriptorTable::createDescriptorRessources()
{
    uint32_t swapChainImageSize = m_renderContext.swapChain().size();
    VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    // Frame descriptor
    VkDeviceSize matrixBufferSize = sizeof(MatrixBuffer::BufferData);
    m_frameDescriptors.resize(swapChainImageSize);
    m_ressourceDescriptors.reserve(m_materials.size());

    for (auto& frameDescriptor : m_frameDescriptors) {
        frameDescriptor.initialize(m_materials);

        // global descriptor
        auto& globalDescriptor = frameDescriptor.getGlobalDescriptorEntry();
        m_renderContext.createBuffer(matrixBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, memoryPropertyFlags, globalDescriptor.buffer, globalDescriptor.memory);

        // material descriptor
        for (auto& material : m_materials) {
            auto& descriptorEntry = frameDescriptor.getDescriptorEntry(material->materialId());
            material->createFrameDescriptorBuffer(m_renderContext, descriptorEntry.buffer, descriptorEntry.memory);
        }
    }

    // material ressources
    for (auto& material : m_materials) {
        material->createMaterialRessources(m_renderContext);
    }
}

void DescriptorTable::createDescriptorSets()
{
    uint32_t swapChainImageSize = m_renderContext.swapChain().size();

    /* ------------------------- Create Descriptor ------------------------- */
    for (auto& frameDescriptor : m_frameDescriptors) {

        // Global Uniform Descriptors
        VkDescriptorSetAllocateInfo allocInfo{};
        auto& globalDescriptorEntry = frameDescriptor.getGlobalDescriptorEntry();
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &m_worldDescriptorLayout;
        if (vkAllocateDescriptorSets(m_renderContext.device(), &allocInfo, &globalDescriptorEntry.descriptorSet) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        // Material's descriptors
        for (auto& material : m_materials) {
            // material per frame ressource, cpu/gpu parameters
            DescriptorEntry& descriptorEntry = frameDescriptor.getDescriptorEntry(material->materialId());
            std::vector<VkDescriptorSetLayout> frameDescriptorLayout = { material->frameDescriptorLayout() };
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = m_descriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = frameDescriptorLayout.data();

            if (vkAllocateDescriptorSets(m_renderContext.device(), &allocInfo, &descriptorEntry.descriptorSet) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }
        }
    }

    // Material gpu ressources, textures & buffer
    for (auto& material : m_materials) {

        // material gpu ressources, textures & buffer
        VkDescriptorSet& descriptorSet = m_ressourceDescriptors[material->materialId()];
        std::vector<VkDescriptorSetLayout> ressourceDescriptorLayout = { material->ressourceDescriptorLayout() };
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = ressourceDescriptorLayout.data();

        if (vkAllocateDescriptorSets(m_renderContext.device(), &allocInfo, &descriptorSet) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }
    }

    /* ------------------------- Fill Descriptor ------------------------- */
    updateDescriptorSets();
}

void DescriptorTable::updateDescriptorSets()
{

    std::vector<VkWriteDescriptorSet> descriptorWrites;
    for (auto& frameDescriptor : m_frameDescriptors) {

        auto& globalDescriptorEntry = frameDescriptor.getGlobalDescriptorEntry();
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = globalDescriptorEntry.buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(MatrixBuffer::BufferData);
        VkWriteDescriptorSet descriptorWrite;
        descriptorWrite.pNext = NULL;
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = globalDescriptorEntry.descriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrites.push_back(descriptorWrite);

        // Materials
        for (auto& material : m_materials) {
            DescriptorEntry& descriptorEntry = frameDescriptor.getDescriptorEntry(material->materialId());
            VkDescriptorSet ressourceEntry = m_ressourceDescriptors.at(material->materialId());
            material->updateFrameDescriptorSet(m_renderContext, descriptorEntry.descriptorSet, descriptorEntry.buffer);
            material->updateRessourceDescripotSet(m_renderContext, ressourceEntry);
        }
    }

    vkUpdateDescriptorSets(m_renderContext.device(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}


void DescriptorTable::cleanUp()
{
    vkDestroyDescriptorSetLayout(m_renderContext.device(), m_worldDescriptorLayout, nullptr);
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
    return m_frameDescriptors.at(frameIndex).getGlobalDescriptorEntry();
}

VkDescriptorSetLayout DescriptorTable::worldDescriptorLayout() const
{
    return m_worldDescriptorLayout;
}

std::vector<DescriptorEntry> DescriptorTable::getMaterialFrameDescriptor(MaterialID materialId)
{
    std::vector<DescriptorEntry> descriptors;
    for (auto& frameDescriptor : m_frameDescriptors) {
        auto& descriptorRef = frameDescriptor.getDescriptorEntry(materialId);
        descriptors.push_back(descriptorRef);
    }
    return descriptors;
}

VkDescriptorSet DescriptorTable::getMaterialRessourceDescriptor(MaterialID materialId)
{
    return m_ressourceDescriptors.at(materialId);
}