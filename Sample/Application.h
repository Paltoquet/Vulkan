#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "MatrixBuffer.h"
#include "Quad.h"
#include "Mesh.h"

#include <vector>
#include <optional>

class QueueFamilyIndices {
public:
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

public:
    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

class SwapChainSupportDetails {
public:
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class Application {

public:
    Application();
    void run();
    void setResizeWindow(bool resized);

private:
    void initWindow();
    void initVulkan();
    void createInstance();
    void createSurface();
    void createLogicalDevice();
    void pickPhysicalDevice();
    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createDescriptorLayout();
    void createGraphicsPipeline();
    void createFrameBuffers();
    void createCommandPool();
    void createColorResources();
    void createDepthResources();
    void createTextureImage();
    void createTextureSampler();
    void loadModels();
    void createVertexBuffer();
    void createIndexBuffer();
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSetLayout();
    void createDescriptorSet();
    void createCommandBuffers();
    void createSyncObjects();
    void mainLoop();
    void drawFrame();
    void updateUniformBuffer(uint32_t currentImage);
    void recreateSwapChain();
    void cleanup();
    void cleanupSwapChain();

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void createImage(uint32_t width, uint32_t height, uint32_t mip_levels, VkSampleCountFlagBits nbSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void createTextureImageView();
    void generateMipmaps(VkImage image, VkFormat format, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mip_levels);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mip_levels);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

private:
    std::vector<const char*> getRequiredExtensions();
    void setupDebugMessenger();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    VkResult CreateDebugUtilsMessengerEXT(VkInstance, const VkDebugUtilsMessengerCreateInfoEXT*, const VkAllocationCallbacks*, VkDebugUtilsMessengerEXT*);
    void DestroyDebugUtilsMessengerEXT(VkInstance, VkDebugUtilsMessengerEXT, const VkAllocationCallbacks*);
    bool isDeviceSuitable(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSampleCountFlagBits getMaxUsableSampleCount();
    VkFormat findDepthFormat();
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    bool hasStencilComponent(VkFormat format);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    bool checkExtensionSupport();
    bool checkValidationLayerSupport();
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

private:
    GLFWwindow * m_window;
    VkInstance m_instance;
    VkPhysicalDevice m_physicalDevice;
    VkDevice m_device;
    VkDebugUtilsMessengerEXT m_debugMessenger;

    VkSurfaceKHR m_surface;
    VkSwapchainKHR m_swapChain;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;

    VkFormat m_swapChainImageFormat;
    VkSampleCountFlagBits m_MSAASamples;
    VkExtent2D m_swapChainExtent;
    std::vector<VkImage> m_swapChainImages;
    std::vector<VkImageView> m_swapChainImageViews;
    std::vector<VkFramebuffer> m_swapChainFramebuffers;

    VkRenderPass m_renderPass;
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_graphicsPipeline;
    VkBuffer m_vertexBuffer;
    VkDeviceMemory m_vertexBufferMemory;
    VkBuffer m_indexBuffer;
    VkDeviceMemory m_indexBufferMemory;
    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VkDeviceMemory> m_uniformBuffersMemory;
    VkDescriptorPool m_descriptorPool;
    VkDescriptorSetLayout m_descriptorSetLayout;
    std::vector<VkDescriptorSet> m_descriptorSets;

    VkCommandPool m_commandPool;
    std::vector<VkCommandBuffer> m_commandBuffers;
    size_t m_currentFrame;
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;

    std::vector<VkFence> m_inFlightFences;
    std::vector<VkFence> m_imagesInFlight;
    bool m_framebufferResized = false;

    VkImage m_colorImage;
    VkDeviceMemory m_colorImageMemory;
    VkImageView m_colorImageView;

    VkImage m_depthImage;
    VkDeviceMemory m_depthImageMemory;
    VkImageView m_depthImageView;

    VkImage m_textureImage;
    uint32_t m_mipLevels;
    VkDeviceMemory m_textureImageMemory;
    VkImageView m_textureImageView;
    VkSampler m_textureSampler;
    MatrixBuffer m_matrixBuffer;
    //Quad m_quad;
    Mesh m_mesh;
};
