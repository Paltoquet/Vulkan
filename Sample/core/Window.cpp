#include "Window.h"

#include "Platform.h"

static void window_size_callback(GLFWwindow *window, int width, int height)
{
    if (auto platform = reinterpret_cast<Platform *>(glfwGetWindowUserPointer(window)))
    {
        platform->resize(width, height);
    }
}

/* --------------------------------- Constructors --------------------------------- */

Window::Window(Platform* Platform, int width, int height)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_handle = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(m_handle, Platform);
    glfwSetWindowSizeCallback(m_handle, window_size_callback);
}


Window::~Window()
{
    glfwDestroyWindow(m_handle);
    glfwTerminate();
}

/* --------------------------------- Public methods --------------------------------- */

VkSurfaceKHR Window::create_surface(VkInstance &instance)
{
    if (instance == VK_NULL_HANDLE || !m_handle)
    {
        return VK_NULL_HANDLE;
    }

    VkSurfaceKHR surface;

    VkResult errCode = glfwCreateWindowSurface(instance, m_handle, NULL, &surface);

    if (errCode != VK_SUCCESS)
    {
        return VK_NULL_HANDLE;
    }

    return surface;
}

GLFWwindow* Window::handle()
{
    return m_handle;
}
