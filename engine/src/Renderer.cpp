#include "Renderer.h"

Renderer::Renderer(uint32_t width, uint32_t height)
    // Vulkan instance is initialized automatically
    : window(instance, "Radiator", static_cast<int>(width), static_cast<int>(height)), // init window
      device(instance, window.getSurface()), // init device (GPU)
      resourceManager(device), // init resource manager
      frameManager(window, device), // lastly init frame manager
      launchWidth(width),
      launchHeight(height) {
    // Initialize the descriptor pool object from which descriptors will be allocated
    // The numbers here are just for safety - there will never be this many allocations from the pool
    descriptorPool = hammock::DescriptorPool::Builder(device)
            .setMaxSets(20000)
            .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10000)
            .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 10000)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10000)
            .build();
}
