#pragma once

#include "Window.h"
#include "Engine.h"
#include <utils/CameraController.h>
#include <utils/Camera.h>

#include <memory>
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

class Platform
{
public:
    Platform();
    ~Platform();

public:
    void initialize();
    void mainLoop();
    void cleanUp();
    void resize(int width, int height);
    void mouseMove(double xpos, double ypos);
    void mousePress(double xpos, double ypos);
    void mouseRelease(double xpos, double ypos);
    void mouseScroll(double scrollDelta);

private:
    void createVulkanInstance();
    void setupDebugMessenger();
    VkPhysicalDevice pickPhysicalDevice();

private:
    bool checkValidationLayerSupport();
    bool checkExtensionSupport();
    std::vector<const char*> getRequiredExtensions();
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    SwapChainSupportInfos querySwapChainSupport(VkPhysicalDevice device);
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    VkResult CreateDebugUtilsMessengerEXT(VkInstance, const VkDebugUtilsMessengerCreateInfoEXT*, const VkAllocationCallbacks*, VkDebugUtilsMessengerEXT*);
    void DestroyDebugUtilsMessengerEXT(VkInstance, VkDebugUtilsMessengerEXT, const VkAllocationCallbacks*);

private:
    std::string m_applicationName;
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Engine> m_engine;
    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<CameraController> m_cameraController;
    VkInstance m_instance;
    VkDebugUtilsMessengerEXT m_debugMessenger;
    VkPhysicalDevice m_physicalDevice;
    VkSurfaceKHR m_surface;
    VkExtent2D m_dimension;
    SwapChainSupportInfos m_availableSwapChainInfos;
};

