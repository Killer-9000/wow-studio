#pragma once

#include "data/DBCFile.h"
#include "graphics/IWindow.h"
#include <string_view>

class ToolsHubWindow : public IWindow
{
	ImVec4 ExpansionColour(uint32_t expansionID);
	const char* ExpansionName(uint32_t expansionID);

public:
	ToolsHubWindow(IWindow* leadingWindow, std::string_view clientLocation, WowLocaleEnum locale)
		: IWindow("WoW-Studio : Tools Hub", 1600, 900, SDL_WINDOW_RESIZABLE),
		  m_mapDBC(locale), m_clientLocation(clientLocation), m_locale(locale)
	{
		if (leadingWindow)
			leadingWindow->Close();

		if (!m_mapDBC.LoadFile())
			m_error = "Failed to DBFilesClient\\Map.dbc";
	}

	virtual bool Render() override;

private:

	// Error message
	std::string m_error;

	// Client variables
	std::string m_clientLocation;
	WowLocaleEnum m_locale;

	// Database variables
	MapDBC m_mapDBC;

};