#include "Material.h"


std::mutex Material::materialIndexLock;
std::atomic<MaterialID> Material::materialCounter;

/* -------------------------- Constructors -------------------------- */

Material::Material(VkDevice device, VkShaderModule vertexShader, VkShaderModule fragmentShader):
    m_device(device),
    m_vertexShader(vertexShader),
    m_fragmentShader(fragmentShader)
{
    materialIndexLock.lock();
    m_materialId = materialCounter;
    materialCounter++;
    materialIndexLock.unlock();
}

Material::~Material()
{

}

/* -------------------------- Public methods -------------------------- */

/*
    Create binding for shader Uniforms such as:
        - layout(set = 1, binding = n) uniform MyStruct { ... }
        - the set = 0 is dedicated to the global matrix uniform buffer
*/
void Material::createDescriptorLayouts(RenderContext& renderContext)
{
    VkDescriptorSetLayoutCreateInfo ressourcelayoutInfo{};
    ressourcelayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    ressourcelayoutInfo.bindingCount = static_cast<uint32_t>(m_ressourceDescriptorBindings.size());
    ressourcelayoutInfo.pBindings = m_ressourceDescriptorBindings.data();

    if (vkCreateDescriptorSetLayout(renderContext.device(), &ressourcelayoutInfo, nullptr, &m_ressourceDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    VkDescriptorSetLayoutCreateInfo frameLayoutInfo{};
    frameLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    frameLayoutInfo.bindingCount = static_cast<uint32_t>(m_frameDescriptorBindings.size());
    frameLayoutInfo.pBindings = m_frameDescriptorBindings.data();
    // needs VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME
    //layoutInfo.pNext = m_bindingStrategies.size() > 0 ? m_bindingStrategies.data() : NULL;

    if (vkCreateDescriptorSetLayout(renderContext.device(), &frameLayoutInfo, nullptr, &m_frameDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

/*
    Create the graphic pipeline used by the material used during draw call:
        - vkCmdBindPipeline(m_commandBuffers, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
*/
void Material::createPipeline(RenderContext& renderContext, VkRenderPass renderPass, VkVertexInputBindingDescription bindingDescription, std::array<VkVertexInputAttributeDescription, 3> vertexDescription, VkDescriptorSetLayout globalDescriptorLayout)
{
    // Vertex Shader
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = m_vertexShader;
    vertShaderStageInfo.pName = "main";

    // Fragment Shader
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = m_fragmentShader;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    /* --------------------------------- Shader Bindings --------------------------------- */
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexDescription.size());
    vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    /* --------------------------------- Screen & Viewports --------------------------------- */
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)renderContext.width();
    viewport.height = (float)renderContext.height();
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = renderContext.swapChain().dimension();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = renderContext.multiSamplingSamples();
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    //multisampling.sampleShadingEnable = VK_TRUE; // enable sample shading in the pipeline
    //multisampling.minSampleShading = .2f; // min fraction for sample shading; closer to one is smooth
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    /* --------------------------------- Pipeline States --------------------------------- */
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {}; // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    VkDynamicState dynamicStates[] = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_LINE_WIDTH
    };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    /* --------------------------------- Shader Uniforms --------------------------------- */

    VkDescriptorSetLayout descriptors[3] = { globalDescriptorLayout, m_frameDescriptorSetLayout, m_ressourceDescriptorSetLayout };
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 3;
    pipelineLayoutInfo.pSetLayouts = descriptors;
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(renderContext.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    /* --------------------------------- Pipeline Creation --------------------------------- */
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr; // Optional
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    if (vkCreateGraphicsPipelines(renderContext.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
}

void Material::cleanUp(RenderContext& renderContext)
{
    vkDestroyShaderModule(renderContext.device(), m_vertexShader, nullptr);
    vkDestroyShaderModule(renderContext.device(), m_fragmentShader, nullptr);

    vkDestroyDescriptorSetLayout(renderContext.device(), m_ressourceDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(renderContext.device(), m_frameDescriptorSetLayout, nullptr);
}

void Material::destroyPipeline(RenderContext& renderContext)
{
    vkDestroyPipeline(renderContext.device(), m_pipeline, nullptr);
    vkDestroyPipelineLayout(renderContext.device(), m_pipelineLayout, nullptr);
}

MaterialID Material::materialId() const
{
    return m_materialId;
}

VkPipeline Material::pipeline() const
{
    return m_pipeline;
}

VkPipelineLayout Material::pipelineLayout() const
{
    return m_pipelineLayout;
}

VkDescriptorSetLayout Material::ressourceDescriptorLayout() const
{
    return m_ressourceDescriptorSetLayout;
}

VkDescriptorSetLayout Material::frameDescriptorLayout() const
{
    return m_frameDescriptorSetLayout;
}

std::vector<VkDescriptorSetLayoutBinding>& Material::ressourceDescriptorBindings()
{
    return m_ressourceDescriptorBindings;
}

std::vector<VkDescriptorSetLayoutBinding>& Material::frameDescriptorBindings()
{
    return m_frameDescriptorBindings;
}