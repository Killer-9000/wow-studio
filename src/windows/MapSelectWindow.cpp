#include "MapSelectWindow.h"

#include "windows/MapViewWindow.h"
#include "windows/ToolsHubWindow.h"
#include "graphics/WindowMgr.h"

bool MapSelectWindow::Render()
{
	if (!StartRender())
		return true;

	ImGui::DockSpaceOverViewport();

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Menu"))
		{
			if (ImGui::MenuItem("Exit"))
				m_reopenHub = true;
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	if (ImGui::Begin("main"))
	{
		if (!m_error.size())
		{
			if (ImGui::BeginTable("map_table", 2, ImGuiTableFlags_Borders))
			{
				MapDBC& mapDBC = SClientData.MapDatabase;
				for (uint32_t i = 0; i < mapDBC.GetRecordCount(); i++)
				{
					MapDBC::Record* record = mapDBC.GetRecordAlt(i);
					if (record)
					{
						ImGui::TableNextColumn();

						EExpansion_ToColour((EExpansion)record->ExpansionID);
						ImVec4 colour = ImVec4();
						ImGui::TextColored(colour, "%s", record->MapName.data());
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
			ImGui::TextColored({ 0.9f, 0.1f, 0.1f, 1.0f }, "%s", m_error.c_str());
		}
	}
	ImGui::End();

	EndRender();

	if (m_openMap) new MapViewWindow(this, m_openMapName, m_openMapID);
    if (m_reopenHub) new ToolsHubWindow(this);

	return true;
}
