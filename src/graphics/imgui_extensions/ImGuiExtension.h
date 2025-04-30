#pragma once

#include "graphics/imgui/imgui.h"
#include "graphics/imgui/imgui_internal.h"

namespace ImGuiEx
{
	enum class AnchorArea
	{
		CENTRE = 0x0,
		TOP = 0x1,
		BOTTOM = 0x2,
		LEFT = 0x4,
		RIGHT = 0x8,

		TOP_LEFT = TOP | LEFT,
		TOP_RIGHT = TOP | RIGHT,
		BOTTOM_LEFT = BOTTOM | LEFT,
		BOTTOM_RIGHT = BOTTOM | RIGHT,
	};

	struct Selectable
	{
		char* Label;
		ImGuiID ID;
		ImVec2 CursorPos;
		ImVec2 LastSize;
		bool Selected;
		ImGuiSelectableFlags Flags;

		// Old current selectable.
		int SelectableIndex;
	};

	struct Context : ImGuiContext
	{
		ImPool<Selectable> Selectables;
		Selectable* CurrentSelectable;

		Context(ImFontAtlas* shared_font_atlas = nullptr)
			: ImGuiContext(shared_font_atlas)
		{
			CurrentSelectable = nullptr;
		}
	};

	Context* CreateContext(ImFontAtlas* shared_font_atlas = nullptr);

	void AnchorWindow(ImVec2 parentSize, AnchorArea anchor);

	bool BeginSelectableEx(const char* str_id, ImGuiID id, bool selected = false, ImGuiSelectableFlags flags = 0);

	bool BeginSelectable(const char* str_id, bool selected = false, ImGuiSelectableFlags flags = 0);

	void EndSelectable();
}