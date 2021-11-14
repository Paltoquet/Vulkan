#include "Window.h"

#include "Platform.h"

static void window_size_callback(GLFWwindow *window, int width, int height)
{
    if (auto platform = reinterpret_cast<Platform *>(glfwGetWindowUserPointer(window)))
    {
        platform->resize(width, height);
    }
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (auto platform = reinterpret_cast<Platform *>(glfwGetWindowUserPointer(window)))
    {
        platform->mouseMove(xpos, ypos);
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (auto platform = reinterpret_cast<Platform *>(glfwGetWindowUserPointer(window))) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            platform->mousePress(xpos, ypos);
        }
        else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
            platform->mouseRelease(xpos, ypos);
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (auto platform = reinterpret_cast<Platform *>(glfwGetWindowUserPointer(window))) {
        platform->mouseScroll(yoffset);
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
    glfwSetCursorPosCallback(m_handle, cursor_position_callback);
    glfwSetMouseButtonCallback(m_handle, mouse_button_callback);
    glfwSetScrollCallback(m_handle, scroll_callback);
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
