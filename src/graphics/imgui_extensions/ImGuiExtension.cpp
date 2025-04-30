#include "ImGuiExtension.h"

namespace ImGuiEx
{
	Context* CreateContext(ImFontAtlas* shared_font_atlas)
	{
		ImGuiContext* prev_ctx = ImGui::GetCurrentContext();
		ImGuiContext* ctx = IM_NEW(Context)(shared_font_atlas);
		ImGui::SetCurrentContext(ctx);
		ImGui::Initialize();
		if (prev_ctx != NULL)
			ImGui::SetCurrentContext(prev_ctx); // Restore previous context if any, else keep new one.
		return (Context*)ctx;
	}

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

	bool BeginSelectableEx(const char* str_id, ImGuiID id, bool selected, ImGuiSelectableFlags flags)
	{
		Context& g = *(Context*)GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		if (window->SkipItems)
			return false;

		Selectable* selectable = g.Selectables.GetByKey(id);
		if (selectable)
		{
			free((void*)selectable->Label);
			selectable->Label = strdup(str_id);
			selectable->CursorPos = ImGui::GetCursorPos();
			selectable->Selected = selected;
			selectable->Flags = flags;
		}
		else
		{
			selectable = g.Selectables.GetOrAddByKey(id);
			selectable->Label = strdup(str_id);
			selectable->ID = id;
			selectable->CursorPos = ImGui::GetCursorPos();
			selectable->LastSize = ImVec2(0.0f, 0.0f);
			selectable->Selected = selected;
			selectable->Flags = flags;
		}

		selectable->SelectableIndex = g.CurrentSelectable ? g.Selectables.GetIndex(g.CurrentSelectable) : -1;
		g.CurrentSelectable = selectable;

		ImRect bb(selectable->CursorPos, ImVec2(selectable->CursorPos.x + selectable->LastSize.x, selectable->CursorPos.y + selectable->LastSize.y));

		if (!ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			return false;

		return ImGui::ItemHoverable(bb, selectable->ID, ImGuiItemFlags_None);
	}

	bool BeginSelectable(const char* str_id, bool selected, ImGuiSelectableFlags flags)
	{
		ImGuiID id = ImGui::GetID(str_id);
		return BeginSelectableEx(str_id, id, selected, flags);
	}

	void EndSelectable()
	{
		Context& g = *(Context*)GImGui;
		Selectable* selectable = g.CurrentSelectable;
		if (selectable == NULL)
		{
			IM_ASSERT_USER_ERROR(selectable != NULL, "EndSelectable() call should only be done while in BeginSelectable() scope!");
			return;
		}

		ImVec2 pos = ImGui::GetCursorPos();

		ImGui::SetCursorPos(selectable->CursorPos);
		selectable->LastSize = ImVec2(pos.x - selectable->CursorPos.x, pos.y - selectable->CursorPos.y);
		if (selectable->LastSize.x == 0.0f)
			selectable->LastSize.x = g.CurrentWindow->WorkRect.Max.x - g.CurrentWindow->WorkRect.Min.x;
		if (selectable->LastSize.y == 0.0f)
			selectable->LastSize.y = g.CurrentWindow->WorkRect.Max.y - g.CurrentWindow->WorkRect.Min.y;
		ImGui::Selectable(selectable->Label, selectable->Selected, selectable->Flags, selectable->LastSize);
		ImGui::SetCursorPos(pos);

		//selectable->Clicked = (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered());
		//selectable->DoubleClicked = (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered());

		// Restore old selectable, or clear it.
		g.CurrentSelectable = selectable->SelectableIndex >= 0 ? g.Selectables.GetByIndex(selectable->SelectableIndex) : NULL;
	}
}