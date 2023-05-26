#pragma once

#include <RefCntAutoPtr.hpp>
#include <SwapChain.h>
#include <SDL.h>
#include <imgui_internal.h>

#include <string_view>
#include <vector>

using namespace std::chrono_literals;

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

		if (event.type == SDL_WINDOWEVENT)
		{
			if (event.window.event == SDL_WINDOWEVENT_CLOSE)
				m_shouldClose = true;
			else if (event.window.event == SDL_WINDOWEVENT_MINIMIZED)
				m_minimized = true;
			else if (event.window.event == SDL_WINDOWEVENT_RESTORED)
				m_minimized = false;
			else if (event.window.event == SDL_WINDOWEVENT_RESIZED)
				SDL_GetWindowSize(m_window, &m_windowWidth, &m_windowHeight);
		}
		return true;
	}
	virtual bool Render() = 0;

	bool ShouldClose() { return m_shouldClose; }
	void Close() { m_shouldClose = true; };

	ImGuiContext* GetImGuiContext() { return m_imguiContext; }
	SDL_Window* GetWindow() { return m_window; }

protected:
	IWindow(const std::string& windowName, int width, int height, SDL_WindowFlags extraFlags = SDL_WindowFlags::SDL_WINDOW_SHOWN);

	bool StartRender();
	void EndRender();

	std::string m_windowName;
	int m_windowWidth;
	int m_windowHeight;
	SDL_WindowFlags m_extraFlags;
	bool m_shouldClose = true;
	bool m_minimized = false;

	SDL_Window* m_window = nullptr;
	ImGuiContext* m_imguiContext;

	std::chrono::nanoseconds frameStart = 0ms;
	std::chrono::nanoseconds frameEnd = 1ms;
	int64_t frameDelta = 16;

	Diligent::RefCntAutoPtr<Diligent::ISwapChain> m_swapchain;

	const float m_clearColour[4] = { .2f,.2f, .2f, 1.0f };
};
