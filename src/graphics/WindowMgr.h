#pragma once

#include "IWindow.h"

#include <backends/imgui_impl_sdl3.h>
#include <tracy/Tracy.hpp>

#include <memory>
#include <vector>

class WindowMgr
{
	typedef std::vector<std::shared_ptr<IWindow>> WindowVector;

	WindowMgr() { }
	~WindowMgr() { ClearWindows(); }

public:
	static WindowMgr* Instance()
	{
		static WindowMgr instance;
		return &instance;
	}

	WindowVector::reference AddWindow(IWindow* window)
	{
		WindowVector::reference wnd = m_windows.emplace_back(window);
		wnd->Init();
		return wnd;
	}

	WindowVector::const_iterator GetWindow(IWindow* window)
	{
		WindowVector::const_iterator itr = std::find_if(m_windows.begin(), m_windows.end(),
			[window](WindowVector::reference& _window) {
				return _window.get() == window;
			});
		return itr;
	}
	WindowVector::const_iterator GetWindowFromID(uint32_t windowID)
	{
		SDL_Window* window = SDL_GetWindowFromID(windowID);
		WindowVector::const_iterator itr = std::find_if(m_windows.begin(), m_windows.end(),
			[window](WindowVector::reference& _window) {
				return _window.get()->GetWindow() == window;
			});
		return itr;
	}

	void RemoveWindow(IWindow* window)
	{
		WindowVector::const_iterator itr = std::find_if(m_windows.begin(), m_windows.end(),
			[window](WindowVector::reference& _window) {
				return _window.get() == window;
			});
		m_windows.erase(itr);
	}

	void ClearWindows()
	{
		for (auto& window : m_windows)
			window->Deinit();
		m_windows.clear();
	}

	size_t WindowCount() { return m_windows.size(); }

	void CleanupWindows()
	{
		ZoneScoped;

		for (int i = 0; i < m_windows.size(); i++)
		{
			if (m_windows[i]->ShouldClose())
			{
				ImGui::SetCurrentContext(m_windows[i]->GetImGuiContext());
				m_windows[i]->Deinit();
				m_windows.erase(m_windows.begin() + i);
			}
		}
	}

	void UpdateWindows()
	{
		ZoneScoped;

		for (int i = 0; i < m_windows.size(); i++)
		{
			std::shared_ptr<IWindow> window = m_windows[i];
			if (!window->ShouldClose())
			{
				FrameMarkStart(window->m_windowName.c_str());
				window->_frameStart = SDL_GetTicksNS();
				ImGui::SetCurrentContext(window->GetImGuiContext());
				{
					ZoneScopedN("IWindow::Update");
					window->Update();
				}
				{
					ZoneScopedN("IWindow::Render");
					window->Render();
				}
				window->_frameEnd = SDL_GetTicksNS();
				window->_frameDelta = (window->_frameEnd - window->_frameStart) / 1000000;
				FrameMarkEnd(window->m_windowName.c_str());
			}
		}
	}

	void ProcessSDLEvents()
	{
		ZoneScoped;

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if ((event.type >= SDL_EVENT_WINDOW_FIRST && event.type <= SDL_EVENT_WINDOW_LAST)
			 || (event.type >= SDL_EVENT_KEY_DOWN && event.type <= SDL_EVENT_MOUSE_REMOVED))
			{

				auto window = GetWindowFromID(event.window.windowID);
				if (window == m_windows.end())
					continue;
				ImGui::SetCurrentContext((*window)->GetImGuiContext());
				ImGui_ImplSDL3_ProcessEvent(&event);
				(*window)->ProcessSDLEvent(event);
			}
			else
			{
				for (auto& window : m_windows)
				{
					auto window = GetWindowFromID(event.window.windowID);
					if (window == m_windows.end())
						continue;
					ImGui::SetCurrentContext((*window)->GetImGuiContext());
					ImGui_ImplSDL3_ProcessEvent(&event);
					(*window)->ProcessSDLEvent(event);
				}
			}
		}
	}

private:
	WindowVector m_windows;
};

#define SWindowMgr WindowMgr::Instance()
