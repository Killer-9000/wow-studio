#include "ToolsHubWindow.h"

ImVec4 ToolsHubWindow::ExpansionColour(uint32_t expansionID)
{
	switch (expansionID)
	{
	case 1:  return ImVec4(0.093f, 0.190f, 0.049f, 1.0f);
	case 2:  return ImVec4(0.046f, 0.136f, 0.196f, 1.0f);
	default: return ImVec4(0.9f,   0.9f,   0.9f,   1.0f);
	}
}

const char* ToolsHubWindow::ExpansionName(uint32_t expansionID)
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
	case 10:  return "The War Within";
	default: return "Unknown";
	}
}

bool ToolsHubWindow::Render()
{
	if (!StartRender())
		return true;

	ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_NoTabBar);

	ImGui::Begin("main");
	{
		ImGui::BeginTabBar("Tools");
		ImGui::BeginTabItem("test");
		ImGui::EndTabItem();
		ImGui::EndTabBar();

		ImGui::BeginTable("Maps", 1);
		for (size_t i = 0; i < m_mapDBC.m_records.size(); i++)
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::LabelText("", "%s", m_mapDBC.GetRecordAlt(i)->MapName.data());
		}
		ImGui::EndTable();
	}
	ImGui::End();


	EndRender();

	return true;
}
