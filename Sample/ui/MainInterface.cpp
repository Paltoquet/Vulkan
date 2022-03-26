#include "FogMenu.h"

#include <iostream>
#include <glm/gtc/quaternion.hpp> 

/* --------------------------------- Constructors --------------------------------- */

FogMenu::FogMenu()
{

}

FogMenu::~FogMenu()
{

}

/* --------------------------------- Public Methods --------------------------------- */

void FogMenu::initiliaze(RenderContext* renderContext)
{
    m_guiWindow.Surface = renderContext->surface();
    m_guiWindow.SurfaceFormat = renderContext->swapChain().vkSurfaceFormat();

    // Check for WSI support
    VkBool32 res;
    vkGetPhysicalDeviceSurfaceSupportKHR(renderContext->physicalDevice(), renderContext->graphicQueueIndex, m_guiWindow.Surface, &res);
    if (res != VK_TRUE)
    {
        fprintf(stderr, "Error no WSI support on physical device 0\n");
        exit(-1);
    }


    SetupVulkanWindow(m_guiWindow, surface, w, h);
}