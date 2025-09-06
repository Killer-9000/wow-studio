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
				auto& project = SSettingsFile._settings.projects[projectID];
				strcpy(m_projectName, project.project_name.c_str());
				strcpy(m_projectPath, project.project_path.c_str());
				strcpy(m_clientPath,  project.client_path.c_str());
				m_clientVersion = project.client_version;

				std::string _tmp;
				if (!OpenClient(project.client_path, _tmp))
					m_openMapSelect = true;
			}
		}
	}

	virtual bool Init() override
	{
		printf("yay");
		return IWindow::Init();
	}
	virtual bool Render() override;

	char m_projectName[254];
	char m_projectPath[254];
	char m_clientPath[254];
	int  m_clientVersion;

private:
	int OpenClient(const std::filesystem::path& directory, std::string& errorMsg);

	bool m_openMapSelect = false;
};