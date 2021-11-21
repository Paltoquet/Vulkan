#include "Engine.h"
#include <utils/ShaderLoader.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <array>
#include <chrono>

/* --------------------------------- Constructors --------------------------------- */

Engine::Engine(const VkSurfaceKHR& surface, const VkPhysicalDevice& device):
    m_renderContext(nullptr),
    m_textureLoader(nullptr)
{
    m_renderContext = std::make_unique<RenderContext>(surface, device);
}


Engine::~Engine()
{
}

/* --------------------------------- Public methods --------------------------------- */

void Engine::initialize(const VkExtent2D& dimension, const SwapChainSupportInfos& swapChainSupport)
{
    m_swapChainSupportInfo = swapChainSupport;
    m_windowWidth = dimension.width;
    m_windowHeight = dimension.height;

    m_renderContext->createLogicalDevice();
    m_renderContext->pickGraphicQueue();
    m_renderContext->pickDepthImageFormat();
    m_renderContext->pickSampleCount();
    m_renderContext->createSwapChain(dimension, swapChainSupport);

    createMainRenderPass();
    m_renderContext->createFrameBuffers(m_mainRenderPass);
    m_renderContext->createCommandPool();

    // Models data
    m_textureLoader = std::make_unique<TextureLoader>(m_renderContext.get());
    createDescriptorLayout();
    loadModels();
    loadTextures();
    loadShaders();

    // Pipeline
    createGraphicsPipeline();
    createTextureSampler();

    createMeshBuffers();
    createUniformBuffer();
    createDescriptorPool();
    createDescriptorSet();
    createCommandBuffers();
    createSyncObjects();
}

void Engine::drawFrame(const Camera& camera)
{
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(m_renderContext->device(), m_renderContext->swapChain().vkSwapChain(), UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    const RenderFrame& currentFrame = m_renderContext->getRenderFrame(imageIndex);
    const VkFence& fence = currentFrame.synchronizationFence();
    vkWaitForFences(m_renderContext->device(), 1, &fence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_renderContext->device(), 1, &fence);

    // --------------------------------- Submit command ---------------------------------

    updateUniformBuffer(camera, imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(m_renderContext->graphicsQueue(), 1, &submitInfo, currentFrame.synchronizationFence()) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    // --------------------------------- Presentation --------------------------------- 
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = { m_renderContext->swapChain().vkSwapChain() };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional 

    result = vkQueuePresentKHR(m_renderContext->presentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    m_currentFrame = (m_currentFrame + 1) % m_renderContext->swapChain().size();
}

void Engine::resize(int width, int height, const SwapChainSupportInfos& swapChainSupport)
{
    m_swapChainSupportInfo = swapChainSupport;
    m_windowWidth = width;
    m_windowHeight = height;
    recreateSwapChain();
}

void Engine::cleanUp()
{
    cleanUpSwapchain();

    vkDestroySampler(m_renderContext->device(), m_textureSampler, nullptr);
    m_textureImageView.cleanUp(m_renderContext->device());
    vkDestroyDescriptorSetLayout(m_renderContext->device(), m_sceneDescriptorLayout, nullptr);

    for (size_t i = 0; i < m_imageAvailableSemaphores.size(); i++) {
        vkDestroySemaphore(m_renderContext->device(), m_renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(m_renderContext->device(), m_imageAvailableSemaphores[i], nullptr);
    }

    m_mesh.cleanUp(*m_renderContext);
    vkDestroyShaderModule(m_renderContext->device(), m_vertTextureShader, nullptr);
    vkDestroyShaderModule(m_renderContext->device(), m_fragTextureShader, nullptr);
    
    m_renderContext->cleanUpDevice();
}

void Engine::recreateSwapChain()
{
    // wait all ressources not in used
    vkDeviceWaitIdle(m_renderContext->device());
    cleanUpSwapchain();

    auto extent = VkExtent2D({ m_windowWidth, m_windowHeight });
    m_renderContext->createSwapChain(extent, m_swapChainSupportInfo);
    createMainRenderPass();
    createGraphicsPipeline();
    m_renderContext->createFrameBuffers(m_mainRenderPass);

    createUniformBuffer();
    createDescriptorPool();
    createDescriptorSet();
    createCommandBuffers();
}

void Engine::cleanUpSwapchain()
{
    m_renderContext->cleanUpFrameBuffers();

    vkFreeCommandBuffers(m_renderContext->device(), m_renderContext->commandPool(), static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
    vkDestroyPipeline(m_renderContext->device(), m_mainGraphicsPipeline, nullptr);
    vkDestroyPipelineLayout(m_renderContext->device(), m_mainPipelineLayout, nullptr);
    vkDestroyRenderPass(m_renderContext->device(), m_mainRenderPass, nullptr);

    for (size_t i = 0; i < m_renderContext->swapChain().size(); i++) {
        vkDestroyBuffer(m_renderContext->device(), m_uniformBuffers[i], nullptr);
        vkFreeMemory(m_renderContext->device(), m_uniformBuffersMemory[i], nullptr);
    }
    vkDestroyDescriptorPool(m_renderContext->device(), m_descriptorPool, nullptr);

    m_renderContext->cleanUpSwapChain();
}

RenderContext* Engine::renderContext()
{
    return m_renderContext.get();
}

/* --------------------------------- Private methods --------------------------------- */

void Engine::updateUniformBuffer(const Camera& camera, uint32_t currentImage)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    glm::mat4 recenter = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, -0.5f));
    m_matrixBuffer.buffer.model = camera.arcBallModel() * recenter;
    //m_matrixBuffer.buffer.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * recenter;
    //m_matrixBuffer.buffer.model = glm::mat4(1.0f); // 
    //m_matrixBuffer.buffer.view = glm::lookAt(glm::vec3(0.0f, 0.0f, -3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    //m_matrixBuffer.buffer.view = glm::lookAt(glm::vec3(2.0f, 0.0f, -2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    //m_matrixBuffer.buffer.proj = glm::perspective(glm::radians(45.0f), m_renderContext->width() / (float)m_renderContext->height(), 0.1f, 10.0f);
    //m_matrixBuffer.buffer.proj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 10.0f);
    //m_matrixBuffer.buffer.proj[1][1] = -1.0f;

    m_matrixBuffer.buffer.view = camera.viewMatrix();
    m_matrixBuffer.buffer.proj = camera.projectionMatrix();
    m_matrixBuffer.buffer.proj[1][1] *= -1;

    void* data;
    vkMapMemory(m_renderContext->device(), m_uniformBuffersMemory[currentImage], 0, sizeof(MatrixBuffer::BufferData), 0, &data);
    memcpy(data, &m_matrixBuffer.buffer, sizeof(MatrixBuffer::BufferData));
    vkUnmapMemory(m_renderContext->device(), m_uniformBuffersMemory[currentImage]);
}

void Engine::createMainRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_renderContext->swapChain().currentImageFormat();
    colorAttachment.samples = m_renderContext->multiSamplingSamples();
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = m_renderContext->depthImageFormat();
    depthAttachment.samples = m_renderContext->multiSamplingSamples();
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format = m_renderContext->swapChain().currentImageFormat();
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentResolveRef{};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.pResolveAttachments = &colorAttachmentResolveRef;

    /* Start the RenderPass when the piepeline is ready to write to the color attachment*/
    VkSubpassDependency dependency{};
    // Implicit before and after subpass
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    // the current subpass
    dependency.dstSubpass = 0;
    // which state to wait
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    // which state to wait
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    // when the transition start
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());;
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(m_renderContext->device(), &renderPassInfo, nullptr, &m_mainRenderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void Engine::createDescriptorLayout()
{
    auto matrixLayoutBinding = m_matrixBuffer.descriptorBinding();

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = { matrixLayoutBinding, samplerLayoutBinding };
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(m_renderContext->device(), &layoutInfo, nullptr, &m_sceneDescriptorLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void Engine::loadModels()
{
    m_mesh.load("ressources/models/viking_room.obj");
}

void Engine::loadTextures()
{
    m_textureImageView = m_textureLoader->loadTexture("ressources/textures/viking_room.png", VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void Engine::loadShaders()
{
    m_vertTextureShader = ShaderLoader::loadShader("shaders/vert.spv", m_renderContext->device());
    m_fragTextureShader = ShaderLoader::loadShader("shaders/frag.spv", m_renderContext->device());
}

void Engine::createGraphicsPipeline()
{
    // Vertex Shader
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = m_vertTextureShader;
    vertShaderStageInfo.pName = "main";

    // Fragment Shader
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = m_fragTextureShader;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    /* --------------------------------- Shader Bindings --------------------------------- */
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};

    //auto bindingDescription    = m_quad.getBindingDescription();
    //auto attributeDescriptions = m_quad.getAttributeDescriptions();
    auto bindingDescription = m_mesh.getBindingDescription();
    auto attributeDescriptions = m_mesh.getAttributeDescriptions();

    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    /* --------------------------------- Screen & Viewports --------------------------------- */
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)m_renderContext->width();
    viewport.height = (float)m_renderContext->height();
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = m_renderContext->swapChain().dimension();

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
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = m_renderContext->multiSamplingSamples();
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
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
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
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_sceneDescriptorLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(m_renderContext->device(), &pipelineLayoutInfo, nullptr, &m_mainPipelineLayout) != VK_SUCCESS) {
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
    pipelineInfo.layout = m_mainPipelineLayout;
    pipelineInfo.renderPass = m_mainRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    if (vkCreateGraphicsPipelines(m_renderContext->device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_mainGraphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    //vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
    //vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
}

void Engine::createCommandBuffers()
{
    uint32_t swapChainImageSize = m_renderContext->swapChain().size();
    m_commandBuffers.resize(swapChainImageSize);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_renderContext->commandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();

    if (vkAllocateCommandBuffers(m_renderContext->device(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    for (size_t i = 0; i < m_commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_mainRenderPass;
        renderPassInfo.framebuffer = m_renderContext->frameBuffers()[i];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_renderContext->dimension();
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_mainGraphicsPipeline);

        VkBuffer vertexBuffers[] = { m_mesh.vertexBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(m_commandBuffers[i], m_mesh.indexBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_mainPipelineLayout, 0, 1, &m_descriptorSets[i], 0, nullptr);
        //vkCmdDrawIndexed(m_commandBuffers[i], static_cast<uint32_t>(m_quad.indices().size()), 1, 0, 0, 0);
        vkCmdDrawIndexed(m_commandBuffers[i], static_cast<uint32_t>(m_mesh.indices().size()), 1, 0, 0, 0);

        vkCmdEndRenderPass(m_commandBuffers[i]);

        if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
}

void Engine::createDescriptorPool()
{
    uint32_t swapChainImageSize = m_renderContext->swapChain().size();
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChainImageSize);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChainImageSize);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(swapChainImageSize);

    if (vkCreateDescriptorPool(m_renderContext->device(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void Engine::createDescriptorSet()
{
    uint32_t swapChainImageSize = m_renderContext->swapChain().size();
    std::vector<VkDescriptorSetLayout> layouts(swapChainImageSize, m_sceneDescriptorLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImageSize);
    allocInfo.pSetLayouts = layouts.data();

    m_descriptorSets.resize(swapChainImageSize);
    if (vkAllocateDescriptorSets(m_renderContext->device(), &allocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < swapChainImageSize; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(MatrixBuffer::BufferData);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = m_textureImageView.view();
        imageInfo.sampler = m_textureSampler;

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = m_descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(m_renderContext->device(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void Engine::createSyncObjects()
{
    m_currentFrame = 0;
    m_imageAvailableSemaphores.resize(m_renderContext->swapChain().size());
    m_renderFinishedSemaphores.resize(m_renderContext->swapChain().size());

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < m_renderContext->swapChain().size(); i++) {
        if (vkCreateSemaphore(m_renderContext->device(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_renderContext->device(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ) {

            throw std::runtime_error("failed to create semaphores for a frame!");
        }
    }
}

void Engine::createTextureSampler()
{
    // query the Anisotropy levels
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(m_renderContext->physicalDevice(), &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(m_textureImageView.mipLevels());

    if (vkCreateSampler(m_renderContext->device(), &samplerInfo, nullptr, &m_textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void Engine::createMeshBuffers()
{
    m_mesh.createBuffers(*m_renderContext);
}

void Engine::createUniformBuffer()
{
    VkDeviceSize bufferSize = sizeof(MatrixBuffer::BufferData);

    uint32_t swapChainImageSize = m_renderContext->swapChain().size();
    m_uniformBuffers.resize(swapChainImageSize);
    m_uniformBuffersMemory.resize(swapChainImageSize);

    for (size_t i = 0; i < swapChainImageSize; i++) {
        m_renderContext->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers[i], m_uniformBuffersMemory[i]);
    }
}