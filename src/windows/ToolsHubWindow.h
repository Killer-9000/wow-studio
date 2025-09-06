#pragma once

#include "data/Misc.h"
#include "graphics/IWindow.h"
#include <string_view>

class ToolsHubWindow : public IWindow
{
public:
	ToolsHubWindow(IWindow* leadingWindow)
		: IWindow("WoW-Studio : Tools Hub", 1600, 900, SDL_WINDOW_RESIZABLE)
	{
		if (leadingWindow)
			leadingWindow->Close();
	}

	virtual bool Render() override;

private:

	// Error message
	std::string m_error;
	
	bool m_reopenProjects = false;
	bool m_loadMap = false;
	bool m_loadModel = false;
};