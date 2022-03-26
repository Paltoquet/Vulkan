#pragma once

#include <glm/glm.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>

#include <core/RenderContext.h>
#include <core/Window.h>

class FogMenu
{
public:
    FogMenu();
    ~FogMenu();

public:
    void initialize(Window* window, RenderContext& renderContext, VkRenderPass mainRenderPass);
    void cleanUp(RenderContext& renderContext);

private:
    VkDescriptorPool m_imGUIPool;
};
