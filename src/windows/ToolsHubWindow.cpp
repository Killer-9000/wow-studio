#include "ToolsHubWindow.h"
#include "windows/MapSelectWindow.h"
#include "windows/ProjectSelectWindow.h"
#include "windows/ModelViewerWindow.h"
#include <fmt/printf.h>

bool ToolsHubWindow::Render()
{
	if (!StartRender())
		return true;

	ImGui::DockSpaceOverViewport();//0, nullptr, ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_NoTabBar);

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Menu"))
		{
			if (ImGui::MenuItem("Exit"))
				m_reopenProjects = true;
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	ImGui::Begin("main");
	{
		if (ImGui::Button("Map editor"))
			m_loadMap = true;
		
		if (ImGui::Button("Model Viewer"))
			m_loadModel = true;

	}
	ImGui::End();

	EndRender();

	if (m_reopenProjects) new ProjectSelectWindow(this, false);
	if (m_loadMap) new MapSelectWindow(this);
	if (m_loadModel) new ModelViewerWindow(this);

	return true;
}
