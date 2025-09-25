#pragma once

// Raydiator is using hammock (github.com/elliahu/hammock) as a rendering backend
#include <hammock/hammock.h>


class Renderer final{
    // Vulkan instance
    hammock::VulkanInstance instance{}; // Note the brackets
    // Window class, uses hammock's window class which stands on top of VulkanSurfer lib
    // The prefix here is important when building on linux as the Window collides with X11's Window type
    hammock::Window window;
    // Physical device (GPU)
    hammock::Device device;
    // Resource manages is responsible for resource alloc/release on device
    hammock::ResourceManager resourceManager;
    // Frame manager handles queue submission and contains swap chain abstraction
    hammock::FrameManager frameManager;
    // Descriptor pool is used to allocate descriptor sets and layouts
    std::unique_ptr<hammock::DescriptorPool> descriptorPool;

    // Launch dimensions
    uint32_t launchWidth, launchHeight;

    public:
    Renderer(uint32_t width, uint32_t height);

};

