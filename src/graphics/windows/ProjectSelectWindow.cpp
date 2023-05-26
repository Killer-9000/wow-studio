#include "ProjectSelectWindow.h"

#include "data/Archive/ArchiveMgr.h"
#include "graphics/imgui_extensions/ImFileDialog.h"
#include "graphics/windows/MapSelectWindow.h"
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
	assert(false || "Unkown locale");
}

bool ProjectSelectWindow::OpenClient(const std::filesystem::path& directory, std::string& errorMsg)
{
	// Check everything exists.
	if (!std::filesystem::exists(directory / "Wow.exe"))
	{
		errorMsg = fmt::sprintf("Failed to find 'Wow.exe' in directory '%s'.", directory.string());
		ImGui::OpenPopup("popup");
		return false;
	}
	if (!std::filesystem::exists(directory / "Data"))
	{
		errorMsg = fmt::sprintf("Failed to find 'Data' in directory '%s'.", directory.string());
		ImGui::OpenPopup("popup");
		return false;
	}
	if (!std::filesystem::exists(directory / "WTF"))
	{
		errorMsg = fmt::sprintf("Failed to find 'WTF' in directory '%s'.", directory.string());
		ImGui::OpenPopup("popup");
		return false;
	}
	if (!std::filesystem::exists(directory / "WTF" / "Config.wtf"))
	{
		errorMsg = fmt::sprintf("Failed to find 'WTF/Config.wtf' in directory '%s'.", directory.string());
		return false;
	}

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

bool ProjectSelectWindow::Render()
{
	bool openMapSelect = false;

	if (!StartRender())
		return true;

	static std::string popupmessage = "";

	ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_NoTabBar);

	if (ImGui::Begin("main"))
	{
		static ImVec2 buttonPos = ImVec2(ImGui::GetCurrentContext()->FontSize * 12, ImGui::GetCurrentContext()->FontSize + 12);
		ImGui::SetCursorPos({ ImGui::GetWindowWidth() / 2 - buttonPos.x / 2, ImGui::GetWindowHeight() / 2 - buttonPos.y / 2});
		if (ImGui::Button("Load Client", buttonPos))
			SFILE_DIALOG->Open("filedialog_loadclient", "Open WoW Directory", "", false, true);

		ImGui::SetCursorPos({ ImGui::GetWindowWidth() - 32, 0 });
		if (ImGui::Button("X", { 32, 32 }))
			m_shouldClose = true;
	}
	ImGui::End();

	// Load mpq.
	if (SFILE_DIALOG->IsDone("filedialog_loadclient"))
	{
		if (SFILE_DIALOG->HasResult()) {
			popupmessage = "";
			if (!OpenClient(SFILE_DIALOG->GetResult(), popupmessage))
				ImGui::OpenPopup("popup");
			else
				openMapSelect = true;
		}
		SFILE_DIALOG->Close();
	}

	if (ImGui::BeginPopup("popup"))
	{
		ImGui::Text(popupmessage.c_str());
		ImGui::EndPopup();
	}

	EndRender();

	if (openMapSelect)
		SWindowMgr->AddWindow(new MapSelectWindow(this, LocaleStringToEnum(_locale)));

	openMapSelect = false;

	return true;
}
