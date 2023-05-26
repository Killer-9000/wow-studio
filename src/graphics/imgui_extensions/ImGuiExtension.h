#pragma once

#include "graphics/imgui/imgui.h"

namespace ImGuiEx
{
	enum class AnchorArea
	{
		CENTRE = 0x0,
		TOP    = 0x1,
		BOTTOM = 0x2,
		LEFT   = 0x4,
		RIGHT  = 0x8,

		TOP_LEFT = TOP | LEFT,
		TOP_RIGHT = TOP | RIGHT,
		BOTTOM_LEFT = BOTTOM | LEFT,
		BOTTOM_RIGHT = BOTTOM | RIGHT,
	};

	void AnchorWindow(ImVec2 parentSize, AnchorArea anchor)
	{
		ImVec2 finalPos;

		switch (anchor)
		{
		case AnchorArea::TOP:
			finalPos = ImVec2(parentSize.x / 2 - ImGui::GetWindowWidth(), 0);
			break;
		case AnchorArea::BOTTOM:
			finalPos = ImVec2(parentSize.x / 2 - ImGui::GetWindowWidth(), parentSize.y - ImGui::GetWindowHeight());
			break;
		case AnchorArea::LEFT:
			finalPos = ImVec2(0, parentSize.y / 2 - ImGui::GetWindowHeight());
			break;
		case AnchorArea::RIGHT:
			finalPos = ImVec2(parentSize.x - ImGui::GetWindowWidth(), parentSize.y / 2 - ImGui::GetWindowHeight());
			break;
		case AnchorArea::TOP_LEFT:
			finalPos = ImVec2(0, 0);
			break;
		case AnchorArea::TOP_RIGHT:
			finalPos = ImVec2(parentSize.x - ImGui::GetWindowWidth(), 0);
			break;
		case AnchorArea::BOTTOM_LEFT:
			finalPos = ImVec2(0, parentSize.y - ImGui::GetWindowHeight());
			break;
		case AnchorArea::BOTTOM_RIGHT:
			finalPos = ImVec2(parentSize.x - ImGui::GetWindowWidth(), parentSize.y - ImGui::GetWindowHeight());
			break;
		default:
			finalPos = ImVec2(parentSize.x / 2 - ImGui::GetWindowWidth(), parentSize.y / 2 - ImGui::GetWindowHeight());
		}

		ImGui::SetWindowPos(finalPos, ImGuiCond_Appearing);
	}
}