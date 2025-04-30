#pragma once

#include "graphics/IWindow.h"
#include "data/archive/MPQArchive.h"
#include "data/DBCFile.h"
#include "data/SettingsFile.h"
#include <filesystem>

class ProjectSelectWindow : public IWindow
{
public:
	ProjectSelectWindow(IWindow* leadingWindow, bool autoLoadProject)
		: IWindow("WoW-Studio", 1600, 900)
	{
		if (leadingWindow)
			leadingWindow->Close();

		if (autoLoadProject)
		{
			int projectID = SSettingsFile._settings.curr_project;
			if (projectID != -1 && SSettingsFile._settings.projects.size() > projectID)
			{
				std::string _tmp;
				if (OpenClient(SSettingsFile._settings.projects[projectID].client_path, _tmp))
					_openMapSelect = true;
			}
		}
	}

	virtual bool Render() override;

private:

	WowLocaleEnum LocaleStringToEnum(char locale[4]);
	bool OpenClient(const std::filesystem::path& directory, std::string& errorMsg);

	char _locale[4];
	bool _openMapSelect = false;

	const std::array<const char*, 11> s_expansionNames =
	{
		"Vanilla",
		"The burning crusade",
		"Wrath of the lich king",
		"Cataclysm",
		"Mists of pandaria",
		"Warlords of draenor",
		"Legion",
		"Battle for azeroth",
		"Shadowlands",
		"Dragonflight",
		"The war within"
	};
};