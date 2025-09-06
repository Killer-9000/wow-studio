#pragma once

#include "data/DBCFile.h"
#include "graphics/IWindow.h"

class MapSelectWindow : public IWindow
{
public:
	MapSelectWindow(IWindow* leadingWindow)
		: IWindow("WoW-Studio : Select a map", 1280, 720, SDL_WINDOW_RESIZABLE)
	{
		if (leadingWindow)
			leadingWindow->Close();
	}

	virtual bool Render() override;

private:
	std::string m_error;

    bool m_reopenHub = false;
	bool m_openMap = false;
	uint32_t m_openMapID = UINT32_MAX;
	const char* m_openMapName = nullptr;
};