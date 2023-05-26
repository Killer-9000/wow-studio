#pragma once

#include "data/DBCFile.h"
#include "graphics/IWindow.h"

class MapSelectWindow : public IWindow
{
	ImVec4 ExpansionColour(uint32_t expansionID);
	const char* ExpansionName(uint32_t expansionID);

public:
	MapSelectWindow(IWindow* leadingWindow, WowLocaleEnum locale)
		: IWindow("WoW-Studio : Select a map", 1280, 720, SDL_WINDOW_RESIZABLE), m_mapDBC(locale), m_locale(locale)
	{
		if (leadingWindow)
			leadingWindow->Close();

		if (!m_mapDBC.LoadFile())
			m_error = "Failed to DBFilesClient\\Map.dbc";
	}

	virtual bool Render() override;

private:
	std::string m_error;
	WowLocaleEnum m_locale;
	MapDBC m_mapDBC;
	bool m_openMap = false;
	uint32_t m_openMapID = UINT32_MAX;
	const char* m_openMapName = nullptr;
};