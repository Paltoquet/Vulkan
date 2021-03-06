#include "FogMenu.h"

#include <iostream>
#include <glm/gtc/quaternion.hpp> 

/* --------------------------------- Constructors --------------------------------- */

FogMenu::FogMenu(ViewParams& viewParams):
    m_viewParams(viewParams),
    m_fogScale(viewParams.fogScale()),
    m_noiseSize(viewParams.noiseSize()),
    m_randomSeed(viewParams.randomSeed()),
    m_fogSpeed(viewParams.speed()),
    m_lightAbsorption(viewParams.lightAbsorption()),
    m_densityTreshold(viewParams.densityTreshold()),
    m_lightColor(viewParams.lightColor()),
    m_inScatering(viewParams.inScatering()),
    m_outScatering(viewParams.outScatering()),
    m_phaseFactor(viewParams.phaseFactor()),
    m_phaseOffset(viewParams.phaseOffset())
{

}

FogMenu::~FogMenu()
{

}

/* --------------------------------- Public Methods --------------------------------- */

void FogMenu::initialize(Window* window, RenderContext& renderContext, VkRenderPass mainRenderPass)
{
    // 1: create descriptor pool for IMGUI
    // the size of the pool is very oversize, but it's copied from imgui demo itself.
    VkDescriptorPoolSize pool_sizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    vkCreateDescriptorPool(renderContext.device(), &pool_info, nullptr, &m_imGUIPool);

    // 2 Initialize imGUI Vulkan
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui_ImplGlfw_InitForVulkan(window->handle(), true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = renderContext.vkInstance();
    init_info.PhysicalDevice = renderContext.physicalDevice();
    init_info.Device = renderContext.device();
    init_info.QueueFamily = renderContext.graphicQueueIndex();
    init_info.Queue = renderContext.graphicsQueue();
    //init_info.PipelineCache = g_PipelineCache;
    init_info.DescriptorPool = m_imGUIPool;
    //init_info.Subpass = 0;
    init_info.MinImageCount = renderContext.swapChain().size();
    init_info.ImageCount = renderContext.swapChain().size();
    init_info.MSAASamples = renderContext.multiSamplingSamples();
    //init_info.Allocator = g_Allocator;
    //init_info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&init_info, mainRenderPass);

    // execute a gpu command to upload imgui font textures
    VkCommandBuffer cmd = renderContext.beginSingleTimeCommands();
    ImGui_ImplVulkan_CreateFontsTexture(cmd);
    renderContext.endSingleTimeCommands(cmd);
    
    //clear font textures from cpu data
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void FogMenu::draw(RenderContext& renderContext)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    //ImGui::ShowDemoWindow();

    auto windowSize = ImVec2(renderContext.width() * 0.25f, renderContext.height() * 0.48f);
    auto windowPosition = ImVec2(renderContext.width() - windowSize.x, 0);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Once);
    ImGui::SetNextWindowPos(windowPosition, ImGuiCond_Once);
    ImGui::Begin("Volumetric Fog");
    ImGui::Text("Upload a 3D texture on the GPU");
    //ImGui::Checkbox("Demo Window", &show_demo_window);
    ImGui::SliderFloat("Fog Dimension", &m_fogScale, 1.0f, 8.0f);
    ImGui::SliderFloat("Noise Dimension", &m_noiseSize, 0.1f, 8.0f);
    ImGui::SliderFloat("Random Seed", &m_randomSeed, 1.0f, 42.0f);
    ImGui::Text("Shader Parameters");
    ImGui::SliderFloat("Light Absorption", &m_lightAbsorption, 0.0f, 2.0f);
    ImGui::SliderFloat("Density Treshold", &m_densityTreshold, 0.0f, 1.0f);
    ImGui::SliderFloat("In Scatering", &m_inScatering, 0.0f, 1.0f);
    ImGui::SliderFloat("Out Scatering", &m_outScatering, 0.0f, 1.0f);
    ImGui::SliderFloat("Phase Factor", &m_phaseFactor, 0.0f, 1.0f);
    ImGui::SliderFloat("Phase Offset", &m_phaseOffset, 0.0f, 1.0f);
    ImGui::ColorEdit4("Sun color", (float*)&m_lightColor); // Edit 3 floats representing a color

    //if (ImGui::Button("Button"))  // Buttons return true when clicked (most widgets return true when edited/activated)
    //    counter++;
    //ImGui::SameLine();

    ImGui::NewLine();
    ImGui::Text("Performance: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();

    ImGui::Render();

    m_viewParams.update(m_fogScale, m_noiseSize, m_randomSeed, m_fogSpeed, m_lightAbsorption, m_densityTreshold, 
        m_lightColor, m_inScatering, m_outScatering, m_phaseFactor, m_phaseOffset);
}

void FogMenu::fillCommandBuffer(VkCommandBuffer& cmdBuffer)
{
    // Record dear imgui primitives into command buffer
    ImDrawData* draw_data = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(draw_data, cmdBuffer);
}

void FogMenu::cleanUp(RenderContext& renderContext)
{
    vkDestroyDescriptorPool(renderContext.device(), m_imGUIPool, nullptr);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
}