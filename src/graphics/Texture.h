#pragma once

#include "graphics/Rendering.h"

class Texture
{
public:
    Texture(vk::ImageCreateInfo imageCI, vma::AllocationCreateInfo allocCI)
        : m_imageCI(imageCI), m_allocCI(allocCI)
    { }
    ~Texture() { }

    bool CreateTexture()
    {
        m_image = SRendering._device.createImage(m_imageCI);
        m_memory = SRendering._vmaAllocator.allocateMemoryForImage(m_image, m_allocCI);
    }

    vk::ImageCreateInfo m_imageCI;
    vma::AllocationCreateInfo m_allocCI;

    vk::Image m_image;
    vma::Allocation m_memory;
    vk::ImageView m_imageView;
    vk::DescriptorSet m_descriptorSet;
};
