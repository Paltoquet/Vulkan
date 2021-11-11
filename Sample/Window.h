#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>

class Platform;

class Window
{
public:
    Window(Platform* plateform, int width, int height);
    ~Window();

public:
    VkSurfaceKHR create_surface(VkInstance& instance);
    GLFWwindow* handle();

private:
    GLFWwindow *m_handle = nullptr;

};

