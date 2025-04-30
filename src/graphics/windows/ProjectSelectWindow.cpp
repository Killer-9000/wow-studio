#include "ProjectSelectWindow.h"

#include "data/Archive/ArchiveMgr.h"
#include "graphics/windows/ToolsHubWindow.h"
#include <graphics/WindowMgr.h>

#include <fmt/printf.h>
#include <fstream>

WowLocaleEnum ProjectSelectWindow::LocaleStringToEnum(char locale[4])
{
	if (!memcmp(locale, "enUS", 4)) return enUS;
	else if (!memcmp(locale, "enGB", 4)) return enGB;
	else if (!memcmp(locale, "koKR", 4)) return koKR;
	else if (!memcmp(locale, "frFR", 4)) return frFR;
	else if (!memcmp(locale, "deDE", 4)) return deDE;
	else if (!memcmp(locale, "enCN", 4)) return enCN;
	else if (!memcmp(locale, "zhCN", 4)) return zhCN;
	else if (!memcmp(locale, "enTW", 4)) return enTW;
	else if (!memcmp(locale, "zhTW", 4)) return zhTW;
	else if (!memcmp(locale, "esES", 4)) return esES;
	else if (!memcmp(locale, "esMX", 4)) return esMX;
	else if (!memcmp(locale, "ruRU", 4)) return ruRU;
	else if (!memcmp(locale, "ptPT", 4)) return ptPT;
	else if (!memcmp(locale, "ptBR", 4)) return ptBR;
	else if (!memcmp(locale, "itIT", 4)) return itIT;
	assert(false && "Unkown locale");
}

bool ProjectSelectWindow::OpenClient(const std::filesystem::path& directory, std::string& errorMsg)
{
	// Check everything exists.
	if (!std::filesystem::exists(directory / "Wow.exe"))
		errorMsg = fmt::sprintf("Failed to find 'Wow.exe' in directory '%s'.", directory.string());
	else if (!std::filesystem::exists(directory / "Data"))
		errorMsg = fmt::sprintf("Failed to find 'Data' in directory '%s'.", directory.string());
	else if (!std::filesystem::exists(directory / "WTF"))
		errorMsg = fmt::sprintf("Failed to find 'WTF' in directory '%s'.", directory.string());
	else if (!std::filesystem::exists(directory / "WTF" / "Config.wtf"))
		errorMsg = fmt::sprintf("Failed to find 'WTF/Config.wtf' in directory '%s'.", directory.string());

	if (!errorMsg.empty())
		return false;

	// Find locale.
	std::ifstream stream;
	stream.open((directory / "WTF" / "Config.wtf"));
	if (stream.is_open())
	{
		std::string line = "";
		while (std::getline(stream, line))
		{
			if (line.find("SET") == line.npos)
				continue;
			if (line.find("locale") == line.npos)
				continue;
			size_t pos = line.find("\"") + 1;
			memcpy(_locale, line.data() + pos, 4);
			break;
		}
		stream.close();
	}

	if (!memcmp(_locale, "\0\0\0\0", 4))
	{
		errorMsg = fmt::sprintf("Failed to find locale from 'Config.wtf' in directory '%s'.", directory.string());
		return false;
	}

	// Load archives in order.
	const std::vector<std::string>& archiveOrder = MPQArchive::GetArchiveLoadOrder();
	for (std::string archiveName : archiveOrder)
	{
		archiveName = (directory / "data" / archiveName).string();

		// Replace locale
		size_t pos = archiveName.find("{Locale}");

		while (pos != archiveName.npos)
		{
			archiveName.replace(pos, 8, _locale, 4);
			pos = archiveName.find("{Locale}");
		}

		std::string archiveName2 = archiveName;
		if ((pos = archiveName.find("{Number}")); pos != archiveName.npos)
		{
			archiveName2.replace(pos, 8, "");
			if (std::filesystem::exists(archiveName2) && !SArchiveMgr->ContainsArchive(archiveName2))
				SArchiveMgr->AddArchive(new MPQArchive(archiveName2));
			for (int i = 1; i < 10; i++)
			{
				archiveName2 = archiveName;
				archiveName2.replace(pos, 8, fmt::sprintf("-%i", i));
				if (std::filesystem::exists(archiveName2) && !SArchiveMgr->ContainsArchive(archiveName2))
					SArchiveMgr->AddArchive(new MPQArchive(archiveName2));
			}
		}
		else if ((pos = archiveName.find("{Letter}")); pos != archiveName.npos)
		{
			archiveName2.replace(pos, 8, "");
			if (std::filesystem::exists(archiveName2) && !SArchiveMgr->ContainsArchive(archiveName2))
				SArchiveMgr->AddArchive(new MPQArchive(archiveName2));
			for (int i = 1; i < 10; i++)
			{
				archiveName2 = archiveName;
				archiveName2.replace(pos, 8, fmt::sprintf("%i", i));
				if (std::filesystem::exists(archiveName2) && !SArchiveMgr->ContainsArchive(archiveName2))
					SArchiveMgr->AddArchive(new MPQArchive(archiveName2));
			}
		}
		else
			if (std::filesystem::exists(archiveName) && !SArchiveMgr->ContainsArchive(archiveName))
				SArchiveMgr->AddArchive(new MPQArchive(archiveName));
	}

	return true;
}

//std::string s_wowDirectory = "";
//void OpenWowFolderCallback(void* userdata, const char* const* filelist, int filter)
//{
//	if (!filelist || !*filelist)
//		return;
//
//	s_wowDirectory = *filelist;
//
//	//assert(false && "Implement this!");
//}

bool ProjectSelectWindow::Render()
{
	static std::string clientLocation;

	if (!StartRender())
		return true;

	//static std::string popupmessage = "";

	ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_NoTabBar);

	static int selectedProject = -1;
	static std::string wowDirectory = "";
	static std::string popupMessage = "";
	if (ImGui::Begin("Projects"))
	{
		const auto& projects = SSettingsFile._settings.projects;
		for (int i = 0; i < projects.size(); i++)
		{
			if (ImGuiEx::BeginSelectable(fmt::sprintf("##project%i", i).c_str(), selectedProject == i))
			{
				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					wowDirectory = projects[i].client_path;
				selectedProject = i;
			}

			ImGui::Text("%s", projects[i].project_name.c_str());
			ImGui::Text("%s", projects[i].project_path.c_str());
			ImGui::Text("%s", projects[i].client_path.c_str());
			ImGui::SameLine();
			ImGuiEx::AnchorWindow(ImGui::GetWindowSize(), ImGuiEx::AnchorArea::RIGHT);
			ImGui::Text("%s", projects[i].client_version < s_expansionNames.size() ? s_expansionNames[projects[i].client_version] : "Unknown version");

			ImGuiEx::EndSelectable();

			ImGui::SetCursorPos(ImVec2( ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 12.0f ));

		}

		ImGui::End();
	}

	if (ImGui::Begin("Menu"))
	{
		static char projectName[254];
		static char projectPath[254];
		static char clientPath[254];
		static int  clientVersion;

		if (ImGui::Button("Create"))
		{
			ImGui::OpenPopup("create_project_popup");
			selectedProject = -1;
			*projectName = '\0';
			*projectPath = '\0';
			*clientPath = '\0';
			clientVersion = 2;
		}

		if (selectedProject == -1)
			ImGui::BeginDisabled();

		if (ImGui::Button("Edit") && selectedProject != -1)
		{
			ImGui::OpenPopup("create_project_popup");
			strcpy(projectName, SSettingsFile._settings.projects[selectedProject].project_name.c_str());
			strcpy(projectPath, SSettingsFile._settings.projects[selectedProject].project_path.c_str());
			strcpy(clientPath, SSettingsFile._settings.projects[selectedProject].client_path.c_str());
			clientVersion = SSettingsFile._settings.projects[selectedProject].client_version;
		}

		if (ImGui::Button("Delete") && selectedProject != -1)
		{
			SSettingsFile._settings.curr_project = -1;
			SSettingsFile._settings.projects.erase(SSettingsFile._settings.projects.begin() + selectedProject);
			SSettingsFile._settings.projects.shrink_to_fit();
		}

		if (selectedProject == -1)
			ImGui::EndDisabled();

		if (ImGui::BeginPopup("create_project_popup"))
		{
			ImGui::Text("Create new project");

			ImGui::InputText("Project name: ", projectName, sizeof(projectName));

			if (ImGui::Button("##project_path_dialog"))
			{
				SDL_ShowOpenFolderDialog([](void* userdata, const char* const* filelist, int filter) -> void {
					if (filelist && *filelist)
						strcpy(projectPath, *filelist);
					}, nullptr, m_window, projectPath, false);
			}
			ImGui::SameLine();
			ImGui::InputText("Project path: ", projectPath, sizeof(projectPath));

			if (ImGui::Button("##client_path_dialog"))
			{
				SDL_ShowOpenFolderDialog([](void* userdata, const char* const* filelist, int filter) -> void {
					if (filelist && *filelist)
						strcpy(clientPath, *filelist);
					}, nullptr, m_window, clientPath, false);
			}
			ImGui::SameLine();
			ImGui::InputText("Client path: ", clientPath, sizeof(clientPath));

			ImGui::Combo("Client Version", &clientVersion, s_expansionNames.data(), s_expansionNames.size(), -1);

			if (ImGui::Button(selectedProject == -1 ? "Create" : "Update"))
			{
				if (selectedProject == -1)
					SSettingsFile._settings.projects.emplace_back(Settings::Project{ std::string(projectName), std::string(projectPath) , std::string(clientPath), (uint32_t)clientVersion });
				else
				{
					SSettingsFile._settings.projects[selectedProject].project_name = projectName;
					SSettingsFile._settings.projects[selectedProject].project_path = projectPath;
					SSettingsFile._settings.projects[selectedProject].client_path = clientPath;
					SSettingsFile._settings.projects[selectedProject].client_version = clientVersion;
				}
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		ImGui::End();
	}

	//if (ImGui::Begin("main"))
	//{
	//	static ImVec2 buttonPos = ImVec2(ImGui::GetCurrentContext()->FontSize * 12, ImGui::GetCurrentContext()->FontSize + 12);
	//	ImGui::SetCursorPos({ ImGui::GetWindowWidth() / 2 - buttonPos.x / 2, ImGui::GetWindowHeight() / 2 - buttonPos.y / 2});
	//	if (ImGui::Button("Load Client", buttonPos))
	//	{
	//		//SFILE_DIALOG->Open("filedialog_loadclient", "Open WoW Directory", "", false, true);
	//		SDL_ShowOpenFolderDialog(OpenWowFolderCallback, nullptr, m_window, "", false);
	//	}

	//	ImGui::SetCursorPos({ ImGui::GetWindowWidth() - 32, 0 });
	//	if (ImGui::Button("X", { 32, 32 }))
	//		m_shouldClose = true;
	//}
	//ImGui::End();

	// Load mpq.
	if (wowDirectory != "")
	{
		popupMessage = "";
		if (!OpenClient(wowDirectory, popupMessage))
			ImGui::OpenPopup("popup");
		else
		{
			_openMapSelect = true;
			clientLocation = wowDirectory;
			wowDirectory = "";
		}
	}

	if (ImGui::BeginPopup("popup"))
	{
		ImGui::Text(popupMessage.c_str());
		ImGui::EndPopup();
	}

	EndRender();

	//if (_openMapSelect)
	//	SWindowMgr->AddWindow(new MapSelectWindow(this, LocaleStringToEnum(_locale)));

	if (_openMapSelect)
	{
		new ToolsHubWindow(this, clientLocation, LocaleStringToEnum(_locale));
	}

	_openMapSelect = false;

	return true;
}
