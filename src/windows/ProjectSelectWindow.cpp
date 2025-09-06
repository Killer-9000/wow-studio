#include "ProjectSelectWindow.h"

#include "data/archive/ArchiveMgr.h"
#include "data/ClientData.h"
#include "windows/ToolsHubWindow.h"
#include <graphics/WindowMgr.h>

#include <fmt/printf.h>
#include <fstream>
#include <regex>

int ProjectSelectWindow::OpenClient(const std::filesystem::path& directory, std::string& errorMsg)
{
	char locale[5] = {0};

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
		return 1;

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
			memcpy(locale, line.data() + pos, 4);
			break;
		}
		stream.close();
	}


	if (!memcmp(locale, "\0\0\0\0", 4))
	{
		for (auto& file : std::filesystem::directory_iterator(directory / "Data"))
		{
			if (!file.is_directory())
				continue;

			for (const char* locale2 : s_localeNames)
				if (file.path().filename() == locale2)
				{
					memcpy(locale, locale2, 4);
					break;
				}
		}
	}

	if (!memcmp(locale, "\0\0\0\0", 4))
	{
		errorMsg = fmt::sprintf("Failed to find locale from 'Config.wtf' in directory '%s'.", directory.string());
		return 1;
	}

	return SArchiveMgr->LoadClientData(directory, (EExpansion)m_clientVersion, ELocale_FromString(locale));

	// // Load archives in order.
	// const std::vector<std::string>& archiveOrder = MPQArchive::GetArchiveLoadOrder((EExpansion)m_clientVersion);
	// for (std::string archiveName : archiveOrder)
	// {
	// 	archiveName = (directory / "Data" / archiveName).string();

	// 	// Replace locale
	// 	size_t pos = archiveName.find("{Locale}");

	// 	while (pos != archiveName.npos)
	// 	{
	// 		archiveName.replace(pos, 8, locale, 4);
	// 		pos = archiveName.find("{Locale}");
	// 	}

	// 	std::string archiveName2 = archiveName;
	// 	if ((pos = archiveName.find("{Number}")); pos != archiveName.npos)
	// 	{
	// 		archiveName2.replace(pos, 8, "");
	// 		for (int i = 1; i < 10; i++)
	// 		{
	// 			archiveName2 = archiveName;
	// 			archiveName2.replace(pos, 8, fmt::sprintf("-%i", i));
	// 			if (std::filesystem::exists(archiveName2) && !SArchiveMgr->ContainsArchive(archiveName2))
	// 				SArchiveMgr->AddArchive(new MPQArchive(archiveName2));
	// 		}
	// 	}
	// 	else if ((pos = archiveName.find("{Letter}")); pos != archiveName.npos)
	// 	{
	// 		archiveName2.replace(pos, 8, "");
	// 		for (int i = 1; i < 10; i++)
	// 		{
	// 			archiveName2 = archiveName;
	// 			archiveName2.replace(pos, 8, fmt::sprintf("%i", i));
	// 			if (std::filesystem::exists(archiveName2) && !SArchiveMgr->ContainsArchive(archiveName2))
	// 				SArchiveMgr->AddArchive(new MPQArchive(archiveName2));
	// 		}
	// 	}
	// 	else if ((pos = archiveName.find("{Version}")); pos != archiveName.npos)
	// 	{
	// 		archiveName2 = archiveName;
	// 		archiveName2.replace(pos, 9, "[0-9]{0,5}");
	// 		auto archivePath = std::filesystem::path(archiveName2);
	// 		auto archiveDir = archivePath.parent_path();
	// 		auto archiveRegex = archivePath.filename();
	// 		for (auto& file : std::filesystem::directory_iterator(archiveDir))
	// 		{
	// 			if (file.exists() && file.is_regular_file() && std::regex_match(file.path().filename().string(), std::regex(archiveRegex.string())) && !SArchiveMgr->ContainsArchive(archiveName2))
	// 				SArchiveMgr->AddArchive(new MPQArchive(file.path().lexically_relative(directory).string()));
	// 		}
	// 	}
	// 	else
	// 	{
	// 		if (std::filesystem::exists(archiveName) && !SArchiveMgr->ContainsArchive(archiveName))
	// 			SArchiveMgr->AddArchive(new MPQArchive(archiveName));
	// 	}
	// }

	// if (SArchiveMgr->GetArchives().size() == 0)
	// 	errorMsg = fmt::sprintf("Failed to load any archives in directory '%s'.", directory.string());
	// else
	// {
	// 	SClientData.Location = directory.string();
	// 	SClientData.Locale = ELocale_FromString(locale);
	// 	assert (SClientData.MapDatabase.LoadFile() && "Failed to load map database from client.");
	// }

	// return SArchiveMgr->GetArchives().size() > 0;
}

bool ProjectSelectWindow::Render()
{
	if (!StartRender())
		return true;

	ImGui::DockSpaceOverViewport();

	static int selectedProject = -1;
	static std::string popupMessage = "";
	bool openClient = false;

	if (ImGui::BeginPopup("popup"))
	{
		ImGui::Text("%s", popupMessage.c_str());

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("create_project_popup"))
	{
		ImGui::Text("Create new project");

		ImGui::InputText("Project name: ", m_projectName, sizeof(m_projectName));

		if (ImGui::Button("##project_path_dialog"))
		{
			SDL_ShowOpenFolderDialog([](void* userdata, const char* const* filelist, int filter) -> void {
				if (filelist && *filelist)
					strcpy(((ProjectSelectWindow*)userdata)->m_projectPath, *filelist);
				}, this, m_window, m_projectPath, false);
		}
		ImGui::SameLine();
		ImGui::InputText("Project path: ", m_projectPath, sizeof(m_projectPath));

		if (ImGui::Button("##client_path_dialog"))
		{
			SDL_ShowOpenFolderDialog([](void* userdata, const char* const* filelist, int filter) -> void {
				if (filelist && *filelist)
					strcpy(((ProjectSelectWindow*)userdata)->m_clientPath, *filelist);
				}, this, m_window, m_clientPath, false);
		}
		ImGui::SameLine();
		ImGui::InputText("Client path: ", m_clientPath, sizeof(m_clientPath));

		// Currently only plan to support MPQ versions.
		ImGui::Combo("Client Version", &m_clientVersion, s_expansionNames.data(), (size_t)EExpansion::MistsOfPandaria + 1, -1);

		if (ImGui::Button(selectedProject == -1 ? "Create" : "Update"))
		{
			if (selectedProject == -1)
				SSettingsFile._settings.projects.emplace_back(Settings::Project{ std::string(m_projectName), std::string(m_projectPath) , std::string(m_clientPath), (uint32_t)m_clientVersion });
			else
			{
				SSettingsFile._settings.projects[selectedProject].project_name = m_projectName;
				SSettingsFile._settings.projects[selectedProject].project_path = m_projectPath;
				SSettingsFile._settings.projects[selectedProject].client_path = m_clientPath;
				SSettingsFile._settings.projects[selectedProject].client_version = m_clientVersion;
			}
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	if (ImGui::Begin("Projects"))
	{
		const auto& projects = SSettingsFile._settings.projects;
		for (int i = 0; i < projects.size(); i++)
		{
			if (ImGuiEx::BeginSelectable(fmt::sprintf("##project%i", i).c_str(), selectedProject == i))
			{
				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					strcpy(m_projectName, projects[i].project_name.c_str());
					strcpy(m_projectPath, projects[i].project_path.c_str());
					strcpy(m_clientPath,  projects[i].client_path.c_str());
					m_clientVersion = projects[i].client_version;

					openClient = true;
				}
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

		if (ImGui::Button("Create"))
		{
			ImGui::OpenPopup("create_project_popup");
			selectedProject = -1;
			*m_projectName = '\0';
			*m_projectPath = '\0';
			*m_clientPath = '\0';
			m_clientVersion = 2;
		}

		if (selectedProject == -1)
			ImGui::BeginDisabled();

		if (ImGui::Button("Edit") && selectedProject != -1)
		{
			ImGui::OpenPopup("create_project_popup");
			strcpy(m_projectName, SSettingsFile._settings.projects[selectedProject].project_name.c_str());
			strcpy(m_projectPath, SSettingsFile._settings.projects[selectedProject].project_path.c_str());
			strcpy(m_clientPath, SSettingsFile._settings.projects[selectedProject].client_path.c_str());
			m_clientVersion = SSettingsFile._settings.projects[selectedProject].client_version;
		}

		if (ImGui::Button("Delete") && selectedProject != -1)
		{
			SSettingsFile._settings.curr_project = -1;
			SSettingsFile._settings.projects.erase(SSettingsFile._settings.projects.begin() + selectedProject);
			SSettingsFile._settings.projects.shrink_to_fit();
		}

		if (selectedProject == -1)
			ImGui::EndDisabled();

		ImGui::End();
	}

	// Load mpq.
	if (openClient)
	{
		popupMessage = "";
		if (OpenClient(m_clientPath, popupMessage))
			ImGui::OpenPopup("popup");
		else
			m_openMapSelect = true;
	}

	EndRender();

	if (m_openMapSelect)
	{
		SSettingsFile._settings.curr_project = selectedProject;
		new ToolsHubWindow(this);
	}

	m_openMapSelect = false;

	return true;
}
