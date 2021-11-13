#include "Image.h"



Image::Image()
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