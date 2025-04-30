#pragma once

#include "graphics/Rendering.h"

#include <SDL3/SDL.h>
#include "graphics/imgui_extensions/ImGuiExtension.h"

#include <string>
#include <vector>

class IWindow
{
	friend class WindowMgr;

public:
	virtual bool Init();
	virtual void Deinit();

	// Update and ProcessSDLEvent are optional.
	virtual bool Update() { return true; }
	virtual bool ProcessSDLEvent(const SDL_Event& event)
	{
		if (event.window.windowID != SDL_GetWindowID(m_window))
			return false;

		if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
			m_shouldClose = true;
		else if (event.type == SDL_EVENT_WINDOW_MINIMIZED)
			m_minimized = true;
		else if (event.type == SDL_EVENT_WINDOW_RESTORED || event.type == SDL_EVENT_WINDOW_MAXIMIZED)
			m_minimized = false;
		else if (event.type == SDL_EVENT_WINDOW_RESIZED)
		{
			SDL_GetWindowSize(m_window, (int*)&m_windowWidth, (int*)&m_windowHeight);
			CreateSwapchain();
		}
		return true;
	}
	virtual bool Render() = 0;

	bool ShouldClose() { return m_shouldClose; }
	void Close() { m_shouldClose = true; };

	ImGuiContext* GetImGuiContext() { return m_imguiContext; }
	SDL_Window* GetWindow() { return m_window; }

protected:
	IWindow(const std::string& windowName, uint32_t width, uint32_t height, SDL_WindowFlags extraFlags = 0);

	bool StartRender();
	void EndRender();

	void CreateSwapchain();

	const vk::CommandBuffer& GetCurrCommandBuffer() const
	{
		return _frameData[_frameIndex].cmdBuffer;
	}

	std::string m_windowName;
	uint32_t m_windowWidth;
	uint32_t m_windowHeight;
	SDL_WindowFlags m_extraFlags;
	bool m_shouldClose = true;
	bool m_minimized = false;

	SDL_Window* m_window = nullptr;
	ImGuiEx::Context* m_imguiContext;

	uint32_t _presentQueueIndex;
	vk::SurfaceKHR _surface;
	vk::SwapchainKHR _swapchain;
	vk::Format _swapchainFormat;
	std::vector<vk::Image> _swapchainImages;
	std::vector<vk::ImageView> _swapchainImageViews;

	static constexpr int FRAMES_IN_FLIGHT = 2;
	uint32_t _frameIndex = 0;
	uint32_t _imageIndex = 0;

	struct FrameData
	{
		vk::CommandBuffer cmdBuffer;
		vk::Semaphore imageAcquiredSemaphore;
		vk::Semaphore renderCompleteSemaphore;
		vk::Fence inFlightFence;
	} _frameData[FRAMES_IN_FLIGHT];

	uint64_t _frameStart = 0;
	uint64_t _frameEnd = 1;
	uint64_t _frameDelta = 16;

	vk::ClearValue _clearColour = vk::ClearValue(vk::ClearColorValue(.2f,.2f, .2f, 1.0f));
};
