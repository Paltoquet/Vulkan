#include "Image.h"

Image::Image():
    Vkimage(VK_NULL_HANDLE),
    Vkmemory(VK_NULL_HANDLE)
{

}


Image::~Image()
{

}


/* --------------------------------- Public methods --------------------------------- */

void Image::cleanUp(VkDevice device)
{
    vkDestroyImage(device, Vkimage, nullptr);
    vkFreeMemory(device, Vkmemory, nullptr);
}