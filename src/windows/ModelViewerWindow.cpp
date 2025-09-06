#include "ModelViewerWindow.h"
#include "windows/ToolsHubWindow.h"
#include "backends/imgui_impl_vulkan.h"
#include <backends/imgui_impl_sdl3.h>

bool ModelViewerWindow::Init()
{
	if (!IWindow::Init())
		return false;

	for (int i = 0; i < FRAMES_IN_FLIGHT; i++)
	{
		auto& frameData = m_viewport.frameData[i];

		frameData.cmdBuffer = SRendering._device.allocateCommandBuffers(vk::CommandBufferAllocateInfo(SRendering._commandPool, vk::CommandBufferLevel::ePrimary, 1)).front();
		frameData.renderCompleteSemaphore = SRendering._device.createSemaphore(vk::SemaphoreCreateInfo());
		frameData.inFlightFence = SRendering._device.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
	}

	m_worldView.worldRenderer.Upload();

	m_fileBrowser.Load();

	return true;
}

void ModelViewerWindow::Deinit()
{
	m_fileBrowser.Unload();
	
	m_worldView.worldRenderer.Unload();

	for (int i = 0; i < FRAMES_IN_FLIGHT; i++)
	{
		auto& frameData = m_viewport.frameData[i];

		SRendering._device.destroySemaphore(frameData.renderCompleteSemaphore);
		SRendering._device.destroyFence(frameData.inFlightFence);

		ImGui_ImplVulkan_RemoveTexture(frameData.renderDS);
		SRendering._device.destroySampler(frameData.renderSampler);
		SRendering._device.destroyImageView(frameData.renderImageView);
		SRendering._vmaAllocator.destroyImage(frameData.renderImage, frameData.renderMemory);
	}

	IWindow::Deinit();
}

void ModelViewerWindow::EndRender()
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
}

bool ModelViewerWindow::Render()
{
	if (!StartRender())
		return true;

	ImGui::DockSpaceOverViewport();

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Menu"))
		{
			if (ImGui::MenuItem("Exit"))
				m_reopenHub = true;
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	bool viewportOpen = ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	if (viewportOpen)
	{
		vk::Result result = SRendering._device.waitForFences({ m_viewport.frameData[_frameIndex].inFlightFence }, true, UINT64_MAX);
		if (result != vk::Result::eSuccess)
			return false;
		SRendering._device.resetFences({ m_viewport.frameData[_frameIndex].inFlightFence });

		m_viewport.focused = m_imguiContext->NavWindow == ImGui::GetCurrentWindowRead();
		ImVec2 viewport = ImGui::GetContentRegionAvail();
		ImVec2 padding = ImGui::GetCurrentWindow()->WindowPadding;
		float x = ImGui::GetCursorPosX() - padding.x;
		float y = ImGui::GetCursorPosY() - padding.y;
		ImGui::SetCursorPos({ x, y });
		m_viewport.pos = glm::vec2{ ImGui::GetCurrentWindow()->Pos.x + x, ImGui::GetCurrentWindow()->Pos.y + y };
		glm::vec2 size = { viewport.x + (padding.x * 2), viewport.y + (padding.y * 2) };
		if (size != m_viewport.size)
		{
			m_viewport.size = size;
			if (!CreateViewportTextures())
			{
				m_viewport.size = glm::vec2(0, 0);
				return false;
			}
		}
		ImGui::Image((ImTextureID)(VkDescriptorSet)m_viewport.frameData[_frameIndex].renderDS, { m_viewport.size.x, m_viewport.size.y });

		if (y == 0.0f)
		{
			x += ImGui::GetWindowPos().x;
			y += ImGui::GetWindowPos().y;
			ImU32 col;
			if (ImGui::GetFocusID() == ImGui::GetID("#UNHIDE"))
				col = ImGui::GetColorU32(ImGuiCol_ButtonActive);
			else if (ImGui::GetHoveredID() == ImGui::GetID("#UNHIDE"))
				col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
			else
				col = ImGui::GetColorU32(ImGuiCol_Button);
			ImGui::GetCurrentWindow()->DrawList->AddTriangleFilled({ x, y }, { x + 12, y }, { x, y + 12 }, col);
		}
	}
	ImGui::End();

	if (ImGui::Begin("Filelist"))
	{
		m_fileBrowser.Render([&](const char* filename){
			
		});
		ImGui::End();
	}

	EndRender();

	if (viewportOpen)
		RenderWorld();

	// Present frame.
	{
		vk::PresentInfoKHR presentInfo;
		std::vector<vk::Semaphore> semaphores;
		semaphores.emplace_back(_frameData[_frameIndex].renderCompleteSemaphore);
		if (viewportOpen)
			semaphores.emplace_back(m_viewport.frameData[_frameIndex].renderCompleteSemaphore);

		presentInfo.setWaitSemaphores(semaphores);
		presentInfo.setSwapchains({ _swapchain });
		presentInfo.setImageIndices({ _imageIndex });
		// auto presentInfo = vk::PresentInfoKHR(1, &_frameData[_frameIndex].renderCompleteSemaphore, 1, &_swapchain, &_imageIndex, nullptr);
		vk::Result result = (vk::Result)vkQueuePresentKHR(SRendering._graphicsQueue, presentInfo);

		if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
			CreateSwapchain();
		else if (result != vk::Result::eSuccess)
			throw result;

		_frameIndex = (_frameIndex + 1) % FRAMES_IN_FLIGHT;
	}

    if (m_reopenHub) new ToolsHubWindow(this);

	return true;
}

bool ModelViewerWindow::CreateViewportTextures()
{
	vk::Device& device = SRendering._device;

	for (int i = 0; i < FRAMES_IN_FLIGHT; i++)
	{
		auto& frameData = m_viewport.frameData[i];

		ImGui_ImplVulkan_RemoveTexture(frameData.renderDS);
		device.destroySampler(frameData.renderSampler);
		device.destroyImageView(frameData.renderImageView);
		SRendering._vmaAllocator.destroyImage(frameData.renderImage, frameData.renderMemory);
	}

	vk::ImageCreateInfo imageCI;
	imageCI.imageType = vk::ImageType::e2D;
	imageCI.format = _swapchainFormat;
	imageCI.extent = vk::Extent3D(m_viewport.size.x, m_viewport.size.y, 1);
	imageCI.mipLevels = 1;
	imageCI.arrayLayers = 1;
	imageCI.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;

	vma::AllocationCreateInfo allocCI;
	allocCI.usage = vma::MemoryUsage::eGpuOnly;
	allocCI.requiredFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;

	vk::ImageViewCreateInfo imageViewCI;
	imageViewCI.viewType = vk::ImageViewType::e2D;
	imageViewCI.format = _swapchainFormat;
	imageViewCI.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	imageViewCI.subresourceRange.levelCount = 1;
	imageViewCI.subresourceRange.layerCount = 1;

	vk::SamplerCreateInfo samplerCI;
	samplerCI.magFilter = vk::Filter::eLinear;
	samplerCI.minFilter = vk::Filter::eLinear;
	samplerCI.mipmapMode = vk::SamplerMipmapMode::eLinear;
	samplerCI.addressModeU = vk::SamplerAddressMode::eRepeat;
	samplerCI.addressModeV = vk::SamplerAddressMode::eRepeat;
	samplerCI.addressModeW = vk::SamplerAddressMode::eRepeat;
	samplerCI.minLod = -1000;
	samplerCI.maxLod = 1000;
	
	for (int i = 0; i < FRAMES_IN_FLIGHT; i++)
	{
		auto& frameData = m_viewport.frameData[i];

		vk::Result result = SRendering._vmaAllocator.createImage(&imageCI, &allocCI, &frameData.renderImage, &frameData.renderMemory, nullptr);
		assert(result == vk::Result::eSuccess && "Failed to create images for viewport.");

		imageViewCI.image = frameData.renderImage;
		frameData.renderImageView = device.createImageView(imageViewCI);
		frameData.renderSampler = device.createSampler(samplerCI);
		frameData.renderDS = ImGui_ImplVulkan_AddTexture(frameData.renderSampler, frameData.renderImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

    return true;
}

void ModelViewerWindow::RenderWorld()
{
	auto& frameData = m_viewport.frameData[_frameIndex];

	// Reset and record command buffer.
	frameData.cmdBuffer.reset();
	frameData.cmdBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
	
	// Swap image layout for rendering.
	{
		vk::ImageMemoryBarrier imageMemoryBarrier;
		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eNone;
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		imageMemoryBarrier.oldLayout = vk::ImageLayout::eUndefined;
		imageMemoryBarrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
		imageMemoryBarrier.image = frameData.renderImage;
		imageMemoryBarrier.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };

		frameData.cmdBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eBottomOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::DependencyFlags(), { }, { }, { imageMemoryBarrier });
	}

	// Attach image for rendering output, and begin rendering.
	{
		vk::RenderingAttachmentInfo colourAttachment;
		colourAttachment.imageView = frameData.renderImageView;
		colourAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		colourAttachment.loadOp = vk::AttachmentLoadOp::eClear;
		colourAttachment.storeOp = vk::AttachmentStoreOp::eStore;
		colourAttachment.clearValue = vk::ClearValue(vk::ClearColorValue(0.5f, 0.1f, 0.1f, 1.0f));
		frameData.cmdBuffer.beginRendering(vk::RenderingInfo(vk::RenderingFlags(), vk::Rect2D({ 0,0 }, { (uint32_t)m_viewport.size.x, (uint32_t)m_viewport.size.y }), 1, 0, 1, &colourAttachment), SRendering.GetDispatch());

		frameData.cmdBuffer.setViewport(0, { vk::Viewport(0.0f, 0.0f, m_viewport.size.x, m_viewport.size.y, 0.1f, 1.0f) });
		frameData.cmdBuffer.setScissor(0, { vk::Rect2D({ 0, 0 }, { (uint32_t)m_viewport.size.x, (uint32_t)m_viewport.size.y }) });
	}

	// Render world.
	m_worldView.worldRenderer.Render(&m_worldView);

	// End rendering
  	frameData.cmdBuffer.endRendering(SRendering.GetDispatch());

	// Swap image layout for presenting.
	{
		vk::ImageMemoryBarrier imageMemoryBarrier;
		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eNone;
		imageMemoryBarrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
		imageMemoryBarrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
		imageMemoryBarrier.image = frameData.renderImage;
		imageMemoryBarrier.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };

		frameData.cmdBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eBottomOfPipe, vk::DependencyFlags(), { }, { }, { imageMemoryBarrier });
	}

	frameData.cmdBuffer.end();

	// Submit command buffer for processing.
	{
		vk::PipelineStageFlags stageBits = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		vk::SubmitInfo submitInfo;
		// submitInfo.setWaitSemaphores({ _frameData[_frameIndex].imageAcquiredSemaphore });
		// submitInfo.setWaitDstStageMask({ stageBits });
		submitInfo.setCommandBuffers({ frameData.cmdBuffer });
		submitInfo.setSignalSemaphores({ frameData.renderCompleteSemaphore });

		SRendering._graphicsQueue.submit(submitInfo, frameData.inFlightFence);
	}


}