#include "Platform.h"


#include <algorithm>
#include <set>
#include <iterator>
#include <iostream>
#include <stdexcept>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

/* --------------------------------- Constructors --------------------------------- */

Platform::Platform():
    m_applicationName("Volumetric fog"),
    m_window(nullptr),
    m_engine(nullptr),
    m_camera(nullptr),
    m_cameraController(nullptr),
    m_viewParams(nullptr),
    m_dimension({ 1200, 600 })
{

}

Platform::~Platform()
{

}

/* --------------------------------- Public methods --------------------------------- */

void Platform::initialize()
{
    m_window = std::make_unique<Window>(this, m_dimension.width, m_dimension.height);
    createVulkanInstance();
    setupDebugMessenger();

    m_surface = m_window->create_surface(m_instance);
    m_physicalDevice = pickPhysicalDevice();
    m_availableSwapChainInfos = querySwapChainSupport(m_physicalDevice);

    // Looking in the z negatif, just like standard OpenGL we use a right handed coordinate system for all world space computation, glm will flipped z during projection
    m_camera = std::make_unique<Camera>(glm::vec2(m_dimension.width, m_dimension.height), glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 45.0f);
    m_cameraController = std::make_unique<CameraController>(m_camera.get(), m_dimension.width, m_dimension.height);
    m_viewParams = std::make_unique<ViewParams>();

    // Initialiaze RenderPass & FrameBuffers
    m_engine = std::make_unique<Engine>(m_instance, m_surface, m_physicalDevice);
    m_engine->initialize(m_window.get(), m_availableSwapChainInfos, *m_viewParams);
}

void Platform::mainLoop()
{
    while (!glfwWindowShouldClose(m_window->handle())) {
        glfwPollEvents();
        m_engine->drawFrame(*m_camera, * m_viewParams);
    }

    vkDeviceWaitIdle(m_engine->renderContext()->device());
}

void Platform::resize(int width, int height)
{
    int frameWidth, frameHeight;
    glfwGetFramebufferSize(m_window->handle(), &frameWidth, &frameHeight);
    glfwWaitEvents();

    m_availableSwapChainInfos = querySwapChainSupport(m_physicalDevice);
    m_engine->resize(frameWidth, frameHeight, m_availableSwapChainInfos);
    m_camera->resize(glm::vec2(frameWidth, frameHeight));
    m_cameraController->resize(frameWidth, frameHeight);
}

void Platform::mouseMove(double xpos, double ypos)
{
    if (m_cameraController) {
        m_cameraController->mouseMove(xpos, ypos);
    }
}

void Platform::mousePress(double xpos, double ypos)
{
    if (m_cameraController) {
        m_cameraController->mousePress(xpos, ypos);
    }
}

void Platform::mouseRelease(double xpos, double ypos)
{
    if (m_cameraController) {
        m_cameraController->mouseRelease(xpos, ypos);
    }
}

void Platform::mouseScroll(double scrollDelta)
{
    if (m_cameraController) {
        m_cameraController->mouseScroll(scrollDelta);
    }
    /*float currentOpacity = m_engine->cubicFog().fogDensity();
    currentOpacity += scrollDelta / 20.0f;
    m_engine->cubicFog().setFogDensity(currentOpacity);*/
}

void Platform::cleanUp()
{
    m_engine->cleanUp();
    if (RenderContext::enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
    }
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyInstance(m_instance, nullptr);
    m_window.reset();
}

/* --------------------------------- Private methods --------------------------------- */

void Platform::createVulkanInstance()
{
    if (RenderContext::enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }
    if (!checkExtensionSupport()) {
        throw std::runtime_error("extension not supported!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = m_applicationName.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    std::vector<const char*> requiredExtensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

    // Validation Layers
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (RenderContext::enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(RenderContext::validationLayers.size());
        createInfo.ppEnabledLayerNames = RenderContext::validationLayers.data();
        //Add temporary debug messenger
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

void Platform::setupDebugMessenger() {
    if (!RenderContext::enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

VkPhysicalDevice Platform::pickPhysicalDevice()
{
    VkPhysicalDevice result = VK_NULL_HANDLE;
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());
    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            result = device;
            break;
        }
    }

    if (result == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

    return result;
}

bool Platform::checkValidationLayerSupport()
{
    bool result = true;
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : RenderContext::validationLayers) {
        auto layerIt = std::find_if(availableLayers.begin(), availableLayers.end(), [layerName](VkLayerProperties& layerProperty) {
            return strcmp(layerName, layerProperty.layerName) == 0;
        });
        bool layerFound = layerIt != availableLayers.end();
        result &= layerFound;
    }

    return result;
}

bool Platform::checkExtensionSupport()
{
    bool result = true;
    std::vector<const char*> requiredExtensionNames = getRequiredExtensions();

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    std::cout << "available extensions:\n";
    for (const auto& extension : extensions) {
        std::cout << '\t' << extension.extensionName << '\n';
    }

    std::cout << "validation:" << std::endl;
    for (auto index = 0; index < requiredExtensionNames.size(); index++) {
        const char* requiredExtension = requiredExtensionNames.at(index);
        auto extensionIt = std::find_if(extensions.begin(), extensions.end(), [requiredExtension](VkExtensionProperties& extensionIt) {
            return strcmp(requiredExtension, extensionIt.extensionName) == 0;
        });
        bool foundExtension = extensionIt != extensions.end();
        std::cout << '\t' << requiredExtension << " found " << foundExtension << std::endl;
        result &= foundExtension;
    }

    return result;
}

std::vector<const char*> Platform::getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (RenderContext::enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    // update descriptor on the fly
    //extensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);

    return extensions;
}

bool Platform::isDeviceSuitable(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    if (!deviceFeatures.geometryShader) {
        return false;
    }

    bool extensionsSupported = checkDeviceExtensionSupport(device);
    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportInfos swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

bool Platform::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(RenderContext::requiredExtensions.begin(), RenderContext::requiredExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

SwapChainSupportInfos Platform::querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportInfos infos;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &infos.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);

    if (formatCount != 0) {
        infos.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, infos.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        infos.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, infos.presentModes.data());
    }

    return infos;
}

void Platform::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = /*VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |*/ VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = /*VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |*/ VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.flags = 0;
    createInfo.pUserData = nullptr;
    createInfo.pNext = nullptr;
}

VkResult Platform::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void Platform::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}