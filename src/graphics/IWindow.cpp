#include "IWindow.h"

#include "WindowMgr.h"
#include "data/SettingsFile.h"
#include "graphics/imgui/backends/imgui_impl_sdl3.h"
#include "graphics/imgui/backends/imgui_impl_vulkan.h"

#include <tracy/Tracy.hpp>

void ImGui_CheckVKResult(VkResult result)
{
  assert(result == VK_SUCCESS && "Failed ImGui vulkan result check.");
}

bool IWindow::Init()
{
  // Create a window
  m_window = SDL_CreateWindow(m_windowName.data(), m_windowWidth, m_windowHeight, m_extraFlags | SDL_WINDOW_VULKAN);
  if (!m_window)
  {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error during startup", "Failed to create window", nullptr);
    return false;
  }

  VkSurfaceKHR surface;
  if (!SDL_Vulkan_CreateSurface(m_window, SRendering._instance, NULL, &surface))
  {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error during startup", "Failed to create window surface", nullptr);
    return false;
  }
  _surface = vk::SurfaceKHR(surface);

  std::vector<vk::QueueFamilyProperties> qfp = SRendering._physicalDevice.getQueueFamilyProperties();
  _presentQueueIndex = UINT32_MAX;
  for (int i = 0; i < qfp.size(); i++)
  {
    if (qfp[i].queueFlags & vk::QueueFlagBits::eGraphics
      && SRendering._physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), _surface))
    {
      _presentQueueIndex = i;
      break;
    }
  }
  assert(_presentQueueIndex != UINT32_MAX && "Failed to find graphics queue on device.");

  CreateSwapchain();

  for (int i = 0; i < FRAMES_IN_FLIGHT; i++)
  {
    _frameData[i].cmdBuffer = SRendering._device.allocateCommandBuffers(vk::CommandBufferAllocateInfo(SRendering._commandPool, vk::CommandBufferLevel::ePrimary, 1)).front();
    _frameData[i].imageAcquiredSemaphore = SRendering._device.createSemaphore(vk::SemaphoreCreateInfo());
    _frameData[i].renderCompleteSemaphore = SRendering._device.createSemaphore(vk::SemaphoreCreateInfo());
    _frameData[i].inFlightFence = SRendering._device.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
  }

  // Initialize ImGui
  m_imguiContext = ImGuiEx::CreateContext();
  ImGui::SetCurrentContext(m_imguiContext);
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking
  ImGui::StyleColorsDark();

  if (!ImGui_ImplSDL3_InitForVulkan(m_window))
  {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error during startup", "Failed to initialize ImGui platform backend", nullptr);
    return false;
  }

  ImGui_ImplVulkan_InitInfo initInfo = {};
  initInfo.Instance = SRendering._instance;
  initInfo.PhysicalDevice = SRendering._physicalDevice;
  initInfo.Device = SRendering._device;
  initInfo.QueueFamily = SRendering._graphicsQueueIndex;
  initInfo.Queue = SRendering._graphicsQueue;
  initInfo.DescriptorPool = VK_NULL_HANDLE;
  initInfo.RenderPass = VK_NULL_HANDLE;
  initInfo.MinImageCount = 2;
  initInfo.ImageCount = 2;
  initInfo.MSAASamples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
  initInfo.PipelineCache = VK_NULL_HANDLE;
  initInfo.Subpass = 0;
  initInfo.DescriptorPoolSize = 16;
  initInfo.UseDynamicRendering = true;
  initInfo.PipelineRenderingCreateInfo = vk::PipelineRenderingCreateInfo(0, 1, &_swapchainFormat);
  initInfo.Allocator = nullptr;
  initInfo.CheckVkResultFn = ImGui_CheckVKResult;
  initInfo.MinAllocationSize = 1024 * 1024;

  if (!ImGui_ImplVulkan_Init(&initInfo))
  {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error during startup", "Failed to initialize ImGui renderer backend", nullptr);
    return false;
  }

  m_shouldClose = false;

  m_initialized = true;

  return true;
}

void IWindow::Deinit()
{
  m_initialized = false;
  m_shouldClose = true;

  SRendering._device.waitIdle();

  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext(m_imguiContext);

  for (int i = 0; i < FRAMES_IN_FLIGHT; i++)
  {
    SRendering._device.destroySemaphore(_frameData[i].imageAcquiredSemaphore);
    SRendering._device.destroySemaphore(_frameData[i].renderCompleteSemaphore);
    SRendering._device.destroyFence(_frameData[i].inFlightFence);
  }

  for (auto& imageView : _swapchainImageViews)
    SRendering._device.destroyImageView(imageView);
  _swapchainImageViews.clear();
  _swapchainImages.clear();
  SRendering._device.destroySwapchainKHR(_swapchain);

  SDL_Vulkan_DestroySurface(SRendering._instance, _surface, nullptr);

  SDL_DestroyWindow(m_window);
}

IWindow::IWindow(const std::string& windowName, uint32_t width, uint32_t height, SDL_WindowFlags extraFlags)
  : m_windowName{ windowName }, m_windowWidth{ width }, m_windowHeight{ height }, m_extraFlags{ extraFlags }
{
  SWindowMgr->AddWindow(this);
}

bool IWindow::StartRender()
{
  FrameMarkStart(m_windowName.c_str());
  const vk::CommandBuffer& currCmdBuffer = GetCurrCommandBuffer();

  if (m_minimized)
    return false;

  ImGui::SetCurrentContext(m_imguiContext);

  vk::Result result = SRendering._device.waitForFences(_frameData[_frameIndex].inFlightFence, true, UINT64_MAX);
  assert(result == vk::Result::eSuccess && "Waiting for frame fences failed.");

  result = SRendering._device.acquireNextImageKHR(_swapchain, UINT64_MAX, _frameData[_frameIndex].imageAcquiredSemaphore, {}, &_imageIndex);

  if (result == vk::Result::eErrorOutOfDateKHR)
  {
    CreateSwapchain();
    return false;
  }
  assert((result == vk::Result::eSuccess || result == vk::Result::eSuboptimalKHR) && "Acquiring presentation image failed.");

  SRendering._device.resetFences(_frameData[_frameIndex].inFlightFence);

  currCmdBuffer.reset();

  currCmdBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

  // Swap image layout for rendering.
  {
    vk::ImageMemoryBarrier imageMemoryBarrier;
    imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eNone;
    imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
    imageMemoryBarrier.oldLayout = vk::ImageLayout::eUndefined;
    imageMemoryBarrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
    imageMemoryBarrier.image = _swapchainImages[_imageIndex];
    imageMemoryBarrier.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };

    currCmdBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eBottomOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::DependencyFlags(), { }, { }, { imageMemoryBarrier });
  }

  vk::RenderingAttachmentInfo colourAttachment;
  colourAttachment.imageView = _swapchainImageViews[_imageIndex];
  colourAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
  colourAttachment.loadOp = vk::AttachmentLoadOp::eClear;
  colourAttachment.storeOp = vk::AttachmentStoreOp::eStore;
  colourAttachment.clearValue = _clearColour;
  currCmdBuffer.beginRendering(vk::RenderingInfo(vk::RenderingFlags(), vk::Rect2D({ 0,0 }, { m_windowWidth, m_windowHeight }), 1, 0, 1, &colourAttachment), SRendering.GetDispatch());

  currCmdBuffer.setViewport(0, { vk::Viewport(0.0f, 0.0f, (float)m_windowWidth, (float)m_windowHeight, 0.1f, 1.0f) });
  currCmdBuffer.setScissor(0, { vk::Rect2D({ 0, 0 }, { m_windowWidth, m_windowHeight }) });

  // Start frame
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  return true;
}

void IWindow::EndRender()
{
  const vk::CommandBuffer& currCmdBuffer = GetCurrCommandBuffer();

  ImGui::SetCurrentContext(m_imguiContext);

  // Render frame
  ImGui::Render();

  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), currCmdBuffer);

  currCmdBuffer.endRendering(SRendering.GetDispatch());

  // Swap image layout for presenting.
  {
    vk::ImageMemoryBarrier imageMemoryBarrier;
    imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
    imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eNone;
    imageMemoryBarrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
    imageMemoryBarrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
    imageMemoryBarrier.image = _swapchainImages[_imageIndex];
    imageMemoryBarrier.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };

    currCmdBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eBottomOfPipe, vk::DependencyFlags(), { }, { }, { imageMemoryBarrier });
  }

  currCmdBuffer.end();

  vk::PipelineStageFlags stageBits = vk::PipelineStageFlagBits::eColorAttachmentOutput;
  auto submitInfo = vk::SubmitInfo(1, &_frameData[_frameIndex].imageAcquiredSemaphore, &stageBits, 1, &currCmdBuffer, 1, &_frameData[_frameIndex].renderCompleteSemaphore);
  SRendering._graphicsQueue.submit(submitInfo, _frameData[_frameIndex].inFlightFence);

  auto presentInfo = vk::PresentInfoKHR(1, &_frameData[_frameIndex].renderCompleteSemaphore, 1, &_swapchain, &_imageIndex, nullptr);
  vk::Result result = (vk::Result)vkQueuePresentKHR(SRendering._graphicsQueue, presentInfo);

  if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
    CreateSwapchain();
  else
  {
    assert(result == vk::Result::eSuccess && "Presenting frame failed.");
  }

  _frameIndex = (_frameIndex + 1) % FRAMES_IN_FLIGHT;

  FrameMarkEnd(m_windowName.c_str());
}

void IWindow::CreateSwapchain()
{
  vk::Result result = (vk::Result)vkDeviceWaitIdle(SRendering._device);
  if (result == vk::Result::eErrorDeviceLost)
  {
    VkDeviceFaultCountsEXT faultCounts = {};
    faultCounts.sType = VK_STRUCTURE_TYPE_DEVICE_FAULT_COUNTS_EXT;
    VkDeviceFaultInfoEXT faultInfo = {};
    faultInfo.sType = VK_STRUCTURE_TYPE_DEVICE_FAULT_INFO_EXT;

    result = (vk::Result)SRendering._dispatchLoader.vkGetDeviceFaultInfoEXT((VkDevice)SRendering._device, &faultCounts, nullptr);
    assert (result == vk::Result::eSuccess && "Failed to get fault info after device loss.");

    faultInfo.pAddressInfos = new VkDeviceFaultAddressInfoEXT[faultCounts.addressInfoCount];
    faultInfo.pVendorInfos = new VkDeviceFaultVendorInfoEXT[faultCounts.vendorInfoCount];
    faultInfo.pVendorBinaryData = 0;

    result = (vk::Result)SRendering._dispatchLoader.vkGetDeviceFaultInfoEXT(SRendering._device, &faultCounts, &faultInfo);
    assert (result == vk::Result::eSuccess && "Failed to get fault info after device loss.");

    printf("Device loss: %s", faultInfo.description);
  }
  assert(result == vk::Result::eSuccess && "Waiting for idle failed.");

  _frameIndex = 0;
  _imageIndex = 0;

  for (auto& imageView : _swapchainImageViews)
    SRendering._device.destroyImageView(imageView);
  _swapchainImageViews.clear();
  _swapchainImages.clear();
  SRendering._device.destroySwapchainKHR(_swapchain);

  // get the supported VkFormats
  std::vector<vk::SurfaceFormatKHR> formats = SRendering._physicalDevice.getSurfaceFormatsKHR(_surface);
  assert(!formats.empty());
  _swapchainFormat = (formats[0].format == vk::Format::eUndefined) ? vk::Format::eB8G8R8A8Unorm : formats[0].format;

  vk::SurfaceCapabilitiesKHR surfaceCapabilities = SRendering._physicalDevice.getSurfaceCapabilitiesKHR(_surface);
  vk::Extent2D               swapchainExtent;
  if (surfaceCapabilities.currentExtent.width == (std::numeric_limits<uint32_t>::max)())
  {
    // If the surface size is undefined, the size is set to the size of the images requested.
    swapchainExtent.width = std::clamp(m_windowWidth, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
    swapchainExtent.height = std::clamp(m_windowHeight, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
  }
  else
  {
    // If the surface size is defined, the swap chain size must match
    swapchainExtent = surfaceCapabilities.currentExtent;
  }

  vk::SurfaceTransformFlagBitsKHR preTransform = (surfaceCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
    ? vk::SurfaceTransformFlagBitsKHR::eIdentity
    : surfaceCapabilities.currentTransform;

  vk::CompositeAlphaFlagBitsKHR compositeAlpha =
    (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied) ? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied
    : (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied) ? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied
    : (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit) ? vk::CompositeAlphaFlagBitsKHR::eInherit
    : vk::CompositeAlphaFlagBitsKHR::eOpaque;

  vk::SwapchainCreateInfoKHR swapChainCreateInfo;
  swapChainCreateInfo.flags = vk::SwapchainCreateFlagsKHR();
  swapChainCreateInfo.surface = _surface;
  swapChainCreateInfo.minImageCount = std::clamp<uint32_t>(FRAMES_IN_FLIGHT, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
  swapChainCreateInfo.imageFormat = _swapchainFormat;
  swapChainCreateInfo.imageExtent = swapchainExtent;
  swapChainCreateInfo.imageArrayLayers = 1;
  swapChainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
  swapChainCreateInfo.preTransform = preTransform;
  swapChainCreateInfo.compositeAlpha = compositeAlpha;

  if (SSettingsFile._settings.graphics.vsync)
    swapChainCreateInfo.presentMode = vk::PresentModeKHR::eFifo;
  else
    swapChainCreateInfo.presentMode = vk::PresentModeKHR::eImmediate;
  
  swapChainCreateInfo.clipped = true;
  swapChainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
  std::array<uint32_t, 1> queueFamilyIndicies = { _presentQueueIndex };
  swapChainCreateInfo.setQueueFamilyIndices(queueFamilyIndicies);

  _swapchain = SRendering._device.createSwapchainKHR(swapChainCreateInfo);

  _swapchainImages = SRendering._device.getSwapchainImagesKHR(_swapchain);

  _swapchainImageViews.reserve(_swapchainImages.size());
  vk::ImageViewCreateInfo imageViewCreateInfo(vk::ImageViewCreateFlags(), {}, vk::ImageViewType::e2D, _swapchainFormat, {}, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
  for (auto& image : _swapchainImages)
  {
    imageViewCreateInfo.image = image;
    _swapchainImageViews.push_back(SRendering._device.createImageView(imageViewCreateInfo));
  }
}
