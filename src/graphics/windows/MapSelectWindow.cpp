#include "MapSelectWindow.h"

#include "graphics/windows/MapViewWindow.h"
#include "graphics/WindowMgr.h"

ImVec4 MapSelectWindow::ExpansionColour(uint32_t expansionID)
{
	switch (expansionID)
	{
	case 1:  return ImVec4(0.093f, 0.190f, 0.049f, 1.0f);
	case 2:  return ImVec4(0.046f, 0.136f, 0.196f, 1.0f);
	default: return ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
	}
}

const char* MapSelectWindow::ExpansionName(uint32_t expansionID)
{
	switch (expansionID)
	{
	case 0:  return "Classic";
	case 1:  return "The Burning Crusade";
	case 2:  return "Wrath Of The Lichking";
	case 3:  return "Cataclysm";
	case 4:  return "Mists Of Pandaria";
	case 5:  return "Warlords Of Draenor";
	case 6:  return "Legion";
	case 7:  return "Battle For Azeroth";
	case 8:  return "Shadowlands";
	case 9:  return "Dragonflight";
	default: return "Unknown";
	}
}

bool MapSelectWindow::Render()
{
	if (!StartRender())
		return true;

	ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_NoTabBar);

	ImGui::Begin("main");

	if (!m_error.size())
	{
		if (ImGui::BeginTable("map_table", 2, ImGuiTableFlags_Borders))
		{
			for (uint32_t i = 0; i < m_mapDBC.GetRecordCount(); i++)
			{
				MapDBC::Record* record = m_mapDBC.GetRecordAlt(i);
				if (record)
				{
					ImGui::TableNextColumn();
					ImGui::TextColored(ExpansionColour(record->ExpansionID), "%s", record->MapName.data());
					ImGui::TableNextColumn();

					if (ImGui::Button(fmt::sprintf("Load ##map_load_%i", i).c_str()))
					{
						printf("Loading map %i.\n", i);
						m_openMap = true;
						m_openMapID = record->ID;
						m_openMapName = record->MapName.data();
					}

					ImGui::TableNextRow();
				}
			}

			ImGui::EndTable();
		}
	}
	else
	{
		ImGui::TextColored({ 0.9f, 0.1f, 0.1f, 1.0f }, m_error.c_str());
	}

	ImGui::End();

	EndRender();

	if (m_openMap)
		SWindowMgr->AddWindow(new MapViewWindow(this, m_openMapName, m_openMapID, m_locale));

	return true;
}
