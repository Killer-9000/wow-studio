#pragma once

#include "graphics/IWindow.h"
#include "data/Archive/MPQArchive.h"
#include "data/DBCFile.h"

#include <filesystem>

class ProjectSelectWindow : public IWindow
{
public:
	ProjectSelectWindow()
		: IWindow("WoW-Studio : Select a client", 720, 480, SDL_WINDOW_BORDERLESS)
	{ }

	WowLocaleEnum LocaleStringToEnum(char locale[4]);

	bool OpenClient(const std::filesystem::path& directory, std::string& errorMsg);

	virtual bool Render() override;

private:
	HANDLE _archive = nullptr;

	char _locale[4] = { 0 };
};
