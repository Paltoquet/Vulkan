#pragma once

#include <glm/glm.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>

#include <core/RenderContext.h>

class FogMenu
{
public:
    FogMenu();
    ~FogMenu();

public:
    void initiliaze(RenderContext* renderContext);

private:
    ImGui_ImplVulkanH_Window m_guiWindow;

};
