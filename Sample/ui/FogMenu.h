#pragma once

#include "ViewParams.h"

#include <glm/glm.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>

#include <core/RenderContext.h>
#include <core/Window.h>

class FogMenu
{
public:
    FogMenu(ViewParams& viewParams);
    ~FogMenu();

public:
    void initialize(Window* window, RenderContext& renderContext, VkRenderPass mainRenderPass);
    void draw(RenderContext& renderContext);
    void fillCommandBuffer(VkCommandBuffer& cmdBuffer);
    void cleanUp(RenderContext& renderContext);

public:
    float getDimension() const;

private:
    VkDescriptorPool m_imGUIPool;
    ViewParams& m_viewParams;
};
