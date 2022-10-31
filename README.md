# Vulkan Engine

This projects feature a basic Vulkan renderer, using ImGui for the graphic interface. This is an attempt at implementing an object oriented approach for a Vulkan based graphic engine.

The demo feature some raymarching through a 3D volume and some CPU noise generation algorithm for uploading 3D texture on the GPU.

![ScreenShot](https://github.com/Paltoquet/Vulkan/blob/main/Sample/ressources/doc/ScreenShot.JPG?raw=true)) 



The source code is spitted across multiple classes, the main parts:

- **Engine**, for the vulkan instantiation and the allocation of the main renderPass
- **RenderScene**, for creating the **SceneObject** and filling the commands buffer used during rendering 
- **DescriptorTable** for keeping tracks of all the materials and the different pipelines

