// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * Copyright 2020 Stephane Cuillerdier (aka Aiekick)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "ImGuiThemeHelper.h"

#include "Res/CustomFont.h"

#include "Panes/SourcePane.h"
#include "Panes/TargetPane.h"
#include "Gui/ImGuiWidgets.h"

#include "imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

ImGuiThemeHelper::ImGuiThemeHelper()
{
	m_FileTypeInfos[".glsl"] = igfd::FilterInfosStruct(ImVec4(0.1f, 0.1f, 0.5f, 1.0f), ICON_IGFS_FILE_TYPE_TEXT);
	m_FileTypeInfos[".vert"] = m_FileTypeInfos[".glsl"];
	m_FileTypeInfos[".frag"] = m_FileTypeInfos[".glsl"];
	m_FileTypeInfos[".tess"] = m_FileTypeInfos[".glsl"];
	m_FileTypeInfos[".eval"] = m_FileTypeInfos[".glsl"];
	m_FileTypeInfos[".ctrl"] = m_FileTypeInfos[".glsl"];
	m_FileTypeInfos[".geom"] = m_FileTypeInfos[".glsl"];
	ApplyStyleColorsLight();
	ApplyPalette(TextEditor::GetLightPalette());
}

ImGuiThemeHelper::~ImGuiThemeHelper() = default;

void ImGuiThemeHelper::Draw()
{
	if (m_ShowImGuiStyle)
		DrawImGuiStyleEditor(&m_ShowImGuiStyle);
	if (m_ShowImGuiEditorStyle)
		DrawImGuiColorTextEditStyleEditor(&m_ShowImGuiEditorStyle);
}

void ImGuiThemeHelper::DrawMenu()
{
	if (ImGui::BeginMenu("ImGui Themes"))
	{
		if (ImGui::MenuItem("Light (default)")) ApplyStyleColorsLight();
		if (ImGui::MenuItem("Classic")) ApplyStyleColorsClassic();
		if (ImGui::MenuItem("Dark")) ApplyStyleColorsDark();
		
		ImGui::Separator();

		ImGui::MenuItem("Customize", "", &m_ShowImGuiStyle);

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Editor Palettes"))
	{
		if (ImGui::MenuItem("Light (default)")) ApplyPalette(TextEditor::GetLightPalette());
		if (ImGui::MenuItem("Dark")) ApplyPalette(TextEditor::GetDarkPalette());
		if (ImGui::MenuItem("Retro Blue")) ApplyPalette(TextEditor::GetRetroBluePalette());

		ImGui::Separator();

		ImGui::MenuItem("Customize", "", &m_ShowImGuiEditorStyle);

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("File Type Colors"))
	{
		for (auto &it : m_FileTypeInfos)
		{
			if (ImGui::ColorEdit4(it.first.c_str(), &m_FileTypeInfos[it.first].color.x))
			{
				ApplyFileTypeColors();
			}
		}

		ImGui::EndMenu();
	}
}

void ImGuiThemeHelper::DrawImGuiStyleEditor(bool *vOpen)
{
	if (ImGui::Begin("Styles Editor", vOpen))
	{
		// You can pass in a reference ImGuiStyle structure to compare to, revert to and save to (else it compares to an internally stored reference)
		ImGuiStyle& style = ImGui::GetStyle();
		static ImGuiStyle ref_saved_style;
		ImGuiStyle *ref = nullptr;

		// Default to using internal storage as reference
		static bool init = true;
		if (init)
		{
			ref_saved_style = style;
		}
		init = false;
		ref = &ref_saved_style;

		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.50f);

		if (ImGui::ShowStyleSelector("Colors##Selector"))
			ref_saved_style = style;
		ImGui::ShowFontSelector("Fonts##Selector");

		// Simplified Settings
		if (ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f"))
			style.GrabRounding = style.FrameRounding; // Make GrabRounding always the same value as FrameRounding
		{ bool window_border = (style.WindowBorderSize > 0.0f); if (ImGui::Checkbox("WindowBorder", &window_border)) style.WindowBorderSize = window_border ? 1.0f : 0.0f; }
		ImGui::SameLine();
		{ bool frame_border = (style.FrameBorderSize > 0.0f); if (ImGui::Checkbox("FrameBorder", &frame_border)) style.FrameBorderSize = frame_border ? 1.0f : 0.0f; }
		ImGui::SameLine();
		{ bool popup_border = (style.PopupBorderSize > 0.0f); if (ImGui::Checkbox("PopupBorder", &popup_border)) style.PopupBorderSize = popup_border ? 1.0f : 0.0f; }

		// Save/Revert button
		if (ImGui::Button("Save Ref"))
			*ref = ref_saved_style = style;
		ImGui::SameLine();
		if (ImGui::Button("Revert Ref"))
			style = *ref;
		ImGui::SameLine();
		ImGui::HelpMarker("Save/Revert in local non-persistent storage. Default Colors definition are not affected. Use \"Export\" below to save them somewhere.");

		ImGui::Separator();

		if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None))
		{
			if (ImGui::BeginTabItem("Sizes"))
			{
				ImGui::Text("Main");
				ImGui::SliderFloat2("WindowPadding", (float*)&style.WindowPadding, 0.0f, 20.0f, "%.0f");
				ImGui::SliderFloat2("FramePadding", (float*)&style.FramePadding, 0.0f, 20.0f, "%.0f");
				ImGui::SliderFloat2("ItemSpacing", (float*)&style.ItemSpacing, 0.0f, 20.0f, "%.0f");
				ImGui::SliderFloat2("ItemInnerSpacing", (float*)&style.ItemInnerSpacing, 0.0f, 20.0f, "%.0f");
				ImGui::SliderFloat2("TouchExtraPadding", (float*)&style.TouchExtraPadding, 0.0f, 10.0f, "%.0f");
				ImGui::SliderFloat("IndentSpacing", &style.IndentSpacing, 0.0f, 30.0f, "%.0f");
				ImGui::SliderFloat("ScrollbarSize", &style.ScrollbarSize, 1.0f, 20.0f, "%.0f");
				ImGui::SliderFloat("GrabMinSize", &style.GrabMinSize, 1.0f, 20.0f, "%.0f");
				ImGui::Text("Borders");
				ImGui::SliderFloat("WindowBorderSize", &style.WindowBorderSize, 0.0f, 1.0f, "%.0f");
				ImGui::SliderFloat("ChildBorderSize", &style.ChildBorderSize, 0.0f, 1.0f, "%.0f");
				ImGui::SliderFloat("PopupBorderSize", &style.PopupBorderSize, 0.0f, 1.0f, "%.0f");
				ImGui::SliderFloat("FrameBorderSize", &style.FrameBorderSize, 0.0f, 1.0f, "%.0f");
				ImGui::SliderFloat("TabBorderSize", &style.TabBorderSize, 0.0f, 1.0f, "%.0f");
				ImGui::Text("Rounding");
				ImGui::SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 12.0f, "%.0f");
				ImGui::SliderFloat("ChildRounding", &style.ChildRounding, 0.0f, 12.0f, "%.0f");
				ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f");
				ImGui::SliderFloat("PopupRounding", &style.PopupRounding, 0.0f, 12.0f, "%.0f");
				ImGui::SliderFloat("ScrollbarRounding", &style.ScrollbarRounding, 0.0f, 12.0f, "%.0f");
				ImGui::SliderFloat("GrabRounding", &style.GrabRounding, 0.0f, 12.0f, "%.0f");
				ImGui::SliderFloat("TabRounding", &style.TabRounding, 0.0f, 12.0f, "%.0f");
				ImGui::Text("Alignment");
				ImGui::SliderFloat2("WindowTitleAlign", (float*)&style.WindowTitleAlign, 0.0f, 1.0f, "%.2f");
				int window_menu_button_position = style.WindowMenuButtonPosition + 1;
				if (ImGui::Combo("WindowMenuButtonPosition", (int*)&window_menu_button_position, "None\0Left\0Right\0"))
					style.WindowMenuButtonPosition = window_menu_button_position - 1;
				ImGui::Combo("ColorButtonPosition", (int*)&style.ColorButtonPosition, "Left\0Right\0");
				ImGui::SliderFloat2("ButtonTextAlign", (float*)&style.ButtonTextAlign, 0.0f, 1.0f, "%.2f"); ImGui::SameLine(); ImGui::HelpMarker("Alignment applies when a button is larger than its text content.");
				ImGui::SliderFloat2("SelectableTextAlign", (float*)&style.SelectableTextAlign, 0.0f, 1.0f, "%.2f"); ImGui::SameLine(); ImGui::HelpMarker("Alignment applies when a selectable is larger than its text content.");
				ImGui::Text("Safe Area Padding"); ImGui::SameLine(); ImGui::HelpMarker("Adjust if you cannot see the edges of your screen (e.g. on a TV where scaling has not been configured).");
				ImGui::SliderFloat2("DisplaySafeAreaPadding", (float*)&style.DisplaySafeAreaPadding, 0.0f, 30.0f, "%.0f");
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Colors"))
			{
				static int output_dest = 0;
				static bool output_only_modified = true;
				if (ImGui::Button("Export"))
				{
					if (output_dest == 0)
						ImGui::LogToClipboard();
					else
						ImGui::LogToTTY();
					ImGui::LogText("ImVec4* colors = ImGui::GetStyle().Colors;" IM_NEWLINE);
					for (int i = 0; i < ImGuiCol_COUNT; i++)
					{
						const ImVec4& col = style.Colors[i];
						const char* name = ImGui::GetStyleColorName(i);
						if (!output_only_modified || memcmp(&col, &ref->Colors[i], sizeof(ImVec4)) != 0)
							ImGui::LogText("colors[ImGuiCol_%s]%*s= ImVec4(%.2ff, %.2ff, %.2ff, %.2ff);" IM_NEWLINE, name, 23 - (int)strlen(name), "", col.x, col.y, col.z, col.w);
					}
					ImGui::LogFinish();
				}
				ImGui::SameLine(); ImGui::SetNextItemWidth(120); ImGui::Combo("##output_type", &output_dest, "To Clipboard\0To TTY\0");
				ImGui::SameLine(); ImGui::Checkbox("Only Modified Colors", &output_only_modified);

				static ImGuiTextFilter filter;
				filter.Draw("Filter colors", ImGui::GetFontSize() * 16);

				static ImGuiColorEditFlags alpha_flags = 0;
				if (ImGui::RadioButton("Opaque", alpha_flags == 0)) { alpha_flags = 0; } ImGui::SameLine();
				if (ImGui::RadioButton("Alpha", alpha_flags == ImGuiColorEditFlags_AlphaPreview)) { alpha_flags = ImGuiColorEditFlags_AlphaPreview; } ImGui::SameLine();
				if (ImGui::RadioButton("Both", alpha_flags == ImGuiColorEditFlags_AlphaPreviewHalf)) { alpha_flags = ImGuiColorEditFlags_AlphaPreviewHalf; } ImGui::SameLine();
				ImGui::HelpMarker("In the color list:\nLeft-click on colored square to open color picker,\nRight-click to open edit options menu.");

				ImGui::BeginChild("##colors", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NavFlattened);
				ImGui::PushItemWidth(-160);
				for (int i = 0; i < ImGuiCol_COUNT; i++)
				{
					const char* name = ImGui::GetStyleColorName(i);
					if (!filter.PassFilter(name))
						continue;
					ImGui::PushID(i);
					ImGui::ColorEdit4("##color", (float*)&style.Colors[i], ImGuiColorEditFlags_AlphaBar | alpha_flags);
					if (memcmp(&style.Colors[i], &ref->Colors[i], sizeof(ImVec4)) != 0)
					{
						// Tips: in a real user application, you may want to merge and use an icon font into the main font, so instead of "Save"/"Revert" you'd use icons.
						// Read the FAQ and docs/FONTS.txt about using icon fonts. It's really easy and super convenient!
						ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Save")) ref->Colors[i] = style.Colors[i];
						ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Revert")) style.Colors[i] = ref->Colors[i];
					}
					ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
					ImGui::TextUnformatted(name);
					ImGui::PopID();
				}
				ImGui::PopItemWidth();
				ImGui::EndChild();

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::PopItemWidth();
	}
	ImGui::End();
}

void ImGuiThemeHelper::DrawImGuiColorTextEditStyleEditor(bool *vOpen)
{
	if (ImGui::Begin("ImGuiColorTextEdit Style Editor", vOpen))
	{
		// You can pass in a reference ImGuiStyle structure to compare to, revert to and save to (else it compares to an internally stored reference)
		TextEditor::Palette& style = GetPalette();
		static TextEditor::Palette ref_saved_style;
		TextEditor::Palette *ref = nullptr;

		// Default to using internal storage as reference
		static bool init = true;
		if (init)
		{
			ref_saved_style = style;
		}
		init = false;
		ref = &ref_saved_style;

		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.50f);

		static int style_idx = -1;
		if (ImGui::Combo("Palette", &style_idx, "Light\0Dark\0Retro Blue\0"))
		{
			switch (style_idx)
			{
			case 0: ApplyPalette(TextEditor::GetLightPalette()); break;
			case 1: ApplyPalette(TextEditor::GetDarkPalette()); break;
			case 2: ApplyPalette(TextEditor::GetRetroBluePalette()); break;
			}
			ref_saved_style = style;
		}

		// Save/Revert button
		if (ImGui::Button("Save Ref"))
			*ref = ref_saved_style = style;
		ImGui::SameLine();
		if (ImGui::Button("Revert Ref"))
			style = *ref;
		ImGui::SameLine();
		ImGui::HelpMarker("Save/Revert in local non-persistent storage. Default Colors definition are not affected. Use \"Export\" below to save them somewhere.");

		ImGui::Separator();

		ImGui::BeginChild("##palette", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NavFlattened);
		ImGui::PushItemWidth(-160);
		for (int i = 0; i < (int)TextEditor::PaletteIndex::Max; i++)
		{
			const char* name = Get_ImGuiColorTextEditPalette_NameFromCol((TextEditor::PaletteIndex)i);
			ImGui::PushID(i);
			ImVec4 col = ImGui::ColorConvertU32ToFloat4(style[i]);
			bool res = ImGui::ColorEdit4("##palettecolor", (float*)&col, ImGuiColorEditFlags_AlphaBar);
			style[i] = ImGui::ColorConvertFloat4ToU32(col);
			if (memcmp(&style[i], &((*ref)[i]), sizeof(ImU32)) != 0)
			{
				// Tips: in a real user application, you may want to merge and use an icon font into the main font, so instead of "Save"/"Revert" you'd use icons.
				// Read the FAQ and docs/FONTS.txt about using icon fonts. It's really easy and super convenient!
				ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);if (ImGui::Button("Save")) (*ref)[i] = style[i];
				ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x); 
				if (ImGui::Button("Revert"))
				{
					style[i] = (*ref)[i];
					res = true;
				}
			}
			if (res)
			{
				ApplyPalette(style);
			}
			ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
			ImGui::TextUnformatted(name);
			ImGui::PopID();
		}
		ImGui::PopItemWidth();
		ImGui::EndChild();
	}
	ImGui::End();
}

void ImGuiThemeHelper::ApplyStyleColorsClassic(ImGuiStyle* dst)
{
	ImGui::StyleColorsClassic(dst);

	ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
	
	style->WindowPadding = ImVec2(4, 4);
	style->FramePadding = ImVec2(4, 4);
	style->ItemSpacing = ImVec2(4, 4);
	style->ItemInnerSpacing = ImVec2(4, 4);
	style->IndentSpacing = 10;
	style->ScrollbarSize = 20;
	style->GrabMinSize = 4;

	style->WindowRounding = 0;
	style->ChildRounding = 0;
	style->FrameRounding = 0;
	style->PopupRounding = 0;
	style->ScrollbarRounding = 0;
	style->GrabRounding = 0;
	style->TabRounding = 0;

	style->WindowBorderSize = 0;
	style->ChildBorderSize = 0;
	style->PopupBorderSize = 0;
	style->FrameBorderSize = 1;
	style->TabBorderSize = 0;

	m_FileTypeInfos[".glsl"] = igfd::FilterInfosStruct(ImVec4(0.1f, 0.1f, 0.5f, 1.0f), ICON_IGFS_FILE_TYPE_TEXT);
	m_FileTypeInfos[".vert"].color = m_FileTypeInfos[".glsl"].color;
	m_FileTypeInfos[".frag"].color = m_FileTypeInfos[".glsl"].color;
	m_FileTypeInfos[".tess"].color = m_FileTypeInfos[".glsl"].color;
	m_FileTypeInfos[".eval"].color = m_FileTypeInfos[".glsl"].color;
	m_FileTypeInfos[".ctrl"].color = m_FileTypeInfos[".glsl"].color;
	m_FileTypeInfos[".geom"].color = m_FileTypeInfos[".glsl"].color;
	
	// dark theme so high color
	goodColor = ImVec4(0.2f, 0.8f, 0.2f, 1.0f);
	badColor = ImVec4(0.8f, 0.2f, 0.2f, 1.0f);

	ApplyFileTypeColors();
}

void ImGuiThemeHelper::ApplyStyleColorsDark(ImGuiStyle* dst)
{
	ImGui::StyleColorsDark(dst);

	ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
	
	style->WindowPadding = ImVec2(4, 4);
	style->FramePadding = ImVec2(4, 4);
	style->ItemSpacing = ImVec2(4, 4);
	style->ItemInnerSpacing = ImVec2(4, 4);
	style->IndentSpacing = 10;
	style->ScrollbarSize = 20;
	style->GrabMinSize = 4;

	style->WindowRounding = 0;
	style->ChildRounding = 0;
	style->FrameRounding = 0;
	style->PopupRounding = 0;
	style->ScrollbarRounding = 0;
	style->GrabRounding = 0;
	style->TabRounding = 0;

	style->WindowBorderSize = 0;
	style->ChildBorderSize = 0;
	style->PopupBorderSize = 0;
	style->FrameBorderSize = 1;
	style->TabBorderSize = 0;

	m_FileTypeInfos[".glsl"] = igfd::FilterInfosStruct(ImVec4(0.1f, 0.1f, 0.5f, 1.0f), ICON_IGFS_FILE_TYPE_TEXT);
	m_FileTypeInfos[".vert"].color = m_FileTypeInfos[".glsl"].color;
	m_FileTypeInfos[".frag"].color = m_FileTypeInfos[".glsl"].color;
	m_FileTypeInfos[".tess"].color = m_FileTypeInfos[".glsl"].color;
	m_FileTypeInfos[".eval"].color = m_FileTypeInfos[".glsl"].color;
	m_FileTypeInfos[".ctrl"].color = m_FileTypeInfos[".glsl"].color;
	m_FileTypeInfos[".geom"].color = m_FileTypeInfos[".glsl"].color;
	
	// dark theme so high color
	goodColor = ImVec4(0.2f, 0.8f, 0.2f, 1.0f);
	badColor = ImVec4(0.8f, 0.2f, 0.2f, 1.0f);

	ApplyFileTypeColors();
}

void ImGuiThemeHelper::ApplyStyleColorsLight(ImGuiStyle* dst)
{
	ImGui::StyleColorsLight(dst);
	
	ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
	
	style->WindowPadding = ImVec2(4, 4);
	style->FramePadding = ImVec2(4, 4);
	style->ItemSpacing = ImVec2(4, 4);
	style->ItemInnerSpacing = ImVec2(4, 4);
	style->IndentSpacing = 10;
	style->ScrollbarSize = 20;
	style->GrabMinSize = 4;

	style->WindowRounding = 0;
	style->ChildRounding = 0;
	style->FrameRounding = 0;
	style->PopupRounding = 0;
	style->ScrollbarRounding = 0;
	style->GrabRounding = 0;
	style->TabRounding = 0;

	style->WindowBorderSize = 0;
	style->ChildBorderSize = 0;
	style->PopupBorderSize = 0;
	style->FrameBorderSize = 1;
	style->TabBorderSize = 0;

	m_FileTypeInfos[".glsl"] = igfd::FilterInfosStruct(ImVec4(0.1f, 0.1f, 0.5f, 1.0f), ICON_IGFS_FILE_TYPE_TEXT);
	m_FileTypeInfos[".vert"].color = m_FileTypeInfos[".glsl"].color;
	m_FileTypeInfos[".frag"].color = m_FileTypeInfos[".glsl"].color;
	m_FileTypeInfos[".tess"].color = m_FileTypeInfos[".glsl"].color;
	m_FileTypeInfos[".eval"].color = m_FileTypeInfos[".glsl"].color;
	m_FileTypeInfos[".ctrl"].color = m_FileTypeInfos[".glsl"].color;
	m_FileTypeInfos[".geom"].color = m_FileTypeInfos[".glsl"].color;
	
	// light theme so low color
	goodColor = ImVec4(0.2f, 0.5f, 0.2f, 1.0f);
	badColor = ImVec4(0.5f, 0.2f, 0.2f, 1.0f);

	ApplyFileTypeColors();
}

void ImGuiThemeHelper::ApplyFileTypeColors()
{
	for (auto &it : m_FileTypeInfos)
	{
		igfd::ImGuiFileDialog::Instance()->SetFilterInfos(it.first, it.second.color, it.second.icon);
	}
}

TextEditor::Palette ImGuiThemeHelper::GetPalette()
{
	return SourcePane::Instance()->GetEditor()->GetPalette();
}

void ImGuiThemeHelper::ApplyPalette(TextEditor::Palette vPalette)
{
	SourcePane::Instance()->GetEditor()->SetPalette(vPalette);
	TargetPane::Instance()->GetEditor()->SetPalette(vPalette);
}

///////////////////////////////////////////////////////
//// CONFIGURATION ////////////////////////////////////
///////////////////////////////////////////////////////

std::string ImGuiThemeHelper::getXml(const std::string& vOffset)
{
	std::string str;

	ImGuiStyle* style = &ImGui::GetStyle();
	ImVec4* colors = style->Colors;
	
	str += vOffset + "<ImGui_Styles>\n";

	for (int i = 0; i < ImGuiCol_COUNT; i++)
	{
		str += vOffset + "\t<" + Get_ImGui_NameFromCol(i) + " value=\"" + ct::fvec4(colors[i]).string() + "\"/>\n";
	}

	str += vOffset + "\t<WindowPadding value=\"" + ct::fvec2(style->WindowPadding).string() + "\"/>\n";
	str += vOffset + "\t<FramePadding value=\"" + ct::fvec2(style->FramePadding).string() + "\"/>\n";
	str += vOffset + "\t<ItemSpacing value=\"" + ct::fvec2(style->ItemSpacing).string() + "\"/>\n";
	str += vOffset + "\t<ItemInnerSpacing value=\"" + ct::fvec2(style->ItemInnerSpacing).string() + "\"/>\n";
	str += vOffset + "\t<IndentSpacing value=\"" + ct::toStr(style->IndentSpacing) + "\"/>\n";
	str += vOffset + "\t<ScrollbarSize value=\"" + ct::toStr(style->ScrollbarSize) + "\"/>\n";
	str += vOffset + "\t<GrabMinSize value=\"" + ct::toStr(style->GrabMinSize) + "\"/>\n";
	str += vOffset + "\t<WindowRounding value=\"" + ct::toStr(style->WindowRounding) + "\"/>\n";
	str += vOffset + "\t<ChildRounding value=\"" + ct::toStr(style->ChildRounding) + "\"/>\n";
	str += vOffset + "\t<FrameRounding value=\"" + ct::toStr(style->FrameRounding) + "\"/>\n";
	str += vOffset + "\t<PopupRounding value=\"" + ct::toStr(style->PopupRounding) + "\"/>\n";
	str += vOffset + "\t<ScrollbarRounding value=\"" + ct::toStr(style->ScrollbarRounding) + "\"/>\n";
	str += vOffset + "\t<GrabRounding value=\"" + ct::toStr(style->GrabRounding) + "\"/>\n";
	str += vOffset + "\t<TabRounding value=\"" + ct::toStr(style->TabRounding) + "\"/>\n";
	str += vOffset + "\t<WindowBorderSize value=\"" + ct::toStr(style->WindowBorderSize) + "\"/>\n";
	str += vOffset + "\t<ChildBorderSize value=\"" + ct::toStr(style->ChildBorderSize) + "\"/>\n";
	str += vOffset + "\t<PopupBorderSize value=\"" + ct::toStr(style->PopupBorderSize) + "\"/>\n";
	str += vOffset + "\t<FrameBorderSize value=\"" + ct::toStr(style->FrameBorderSize) + "\"/>\n";
	str += vOffset + "\t<TabBorderSize value=\"" + ct::toStr(style->TabBorderSize) + "\"/>\n";

	str += vOffset + "</ImGui_Styles>\n";
	
	str += vOffset + "<ImGuiColorTextEdit_Styles>\n";

	auto pal = GetPalette();
	for (int i = 0; i < (int)pal.size(); i++)
	{
		str += vOffset + "\t<" + Get_ImGuiColorTextEditPalette_NameFromCol((TextEditor::PaletteIndex)i) + 
			" value=\"" + ct::toStr(pal[i]) + "\"/>\n";
	}

	str += vOffset + "</ImGuiColorTextEdit_Styles>\n";
	
	str += vOffset + "<FileTypes>\n";
	for (auto &it : m_FileTypeInfos)
	{
		str += vOffset + "\t<filetype value=\"" + it.first + "\" color=\"" +
			ct::fvec4(it.second.color).string() + "\"/>\n";
	}
	str += vOffset + "</FileTypes>\n";

	return str;
}

void ImGuiThemeHelper::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent)
{
	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	if (strParentName == "FileTypes")
	{
		std::string fileType;
		std::string color;
		
		for (const tinyxml2::XMLAttribute* attr = vElem->FirstAttribute(); attr != nullptr; attr = attr->Next())
		{
			std::string attName = attr->Name();
			std::string attValue = attr->Value();

			if (attName == "value") fileType = attValue;
			if (attName == "color") color = attValue;
		}

		m_FileTypeInfos[fileType] = ct::toImVec4(ct::fvariant(color).getV4());
		igfd::ImGuiFileDialog::Instance()->SetFilterInfos(fileType, m_FileTypeInfos[fileType]);
	}

	if (strParentName == "ImGui_Styles")
	{
		auto att = vElem->FirstAttribute();
		if (att && std::string(att->Name()) == "value")
		{
			strValue = att->Value();

			ImGuiStyle* style = &ImGui::GetStyle();
			ImVec4* colors = style->Colors;

			if (strName.find("ImGuiCol") != std::string::npos)
			{
				int id = Get_ImGui_ColFromName(strName);
				if (id >= 0)
				{
					colors[id] = ct::toImVec4(ct::fvariant(strValue).getV4());
					return;
				}
			}

			if (strName == "WindowPadding") style->WindowPadding = ct::toImVec2(ct::fvariant(strValue).getV2());
			else if (strName == "FramePadding") style->FramePadding = ct::toImVec2(ct::fvariant(strValue).getV2());
			else if (strName == "ItemSpacing") style->ItemSpacing = ct::toImVec2(ct::fvariant(strValue).getV2());
			else if (strName == "ItemInnerSpacing") style->ItemInnerSpacing = ct::toImVec2(ct::fvariant(strValue).getV2());
			else if (strName == "IndentSpacing") style->IndentSpacing = ct::fvariant(strValue).getF();
			else if (strName == "ScrollbarSize") style->ScrollbarSize = ct::fvariant(strValue).getF();
			else if (strName == "GrabMinSize") style->GrabMinSize = ct::fvariant(strValue).getF();
			else if (strName == "WindowRounding") style->WindowRounding = ct::fvariant(strValue).getF();
			else if (strName == "ChildRounding") style->ChildRounding = ct::fvariant(strValue).getF();
			else if (strName == "FrameRounding") style->FrameRounding = ct::fvariant(strValue).getF();
			else if (strName == "PopupRounding") style->PopupRounding = ct::fvariant(strValue).getF();
			else if (strName == "ScrollbarRounding") style->ScrollbarRounding = ct::fvariant(strValue).getF();
			else if (strName == "GrabRounding") style->GrabRounding = ct::fvariant(strValue).getF();
			else if (strName == "TabRounding") style->TabRounding = ct::fvariant(strValue).getF();
			else if (strName == "WindowBorderSize") style->WindowBorderSize = ct::fvariant(strValue).getF();
			else if (strName == "ChildBorderSize") style->ChildBorderSize = ct::fvariant(strValue).getF();
			else if (strName == "PopupBorderSize") style->PopupBorderSize = ct::fvariant(strValue).getF();
			else if (strName == "FrameBorderSize") style->FrameBorderSize = ct::fvariant(strValue).getF();
			else if (strName == "TabBorderSize") style->TabBorderSize = ct::fvariant(strValue).getF();
		}
	}

	if (strParentName == "ImGuiColorTextEdit_Styles")
	{
		auto att = vElem->FirstAttribute();
		if (att && std::string(att->Name()) == "value")
		{
			strValue = att->Value();

			ImGuiStyle* style = &ImGui::GetStyle();
			ImVec4* colors = style->Colors;

			auto id = Get_ImGuiColorTextEditPalette_ColFromName(strName);
			if (id < TextEditor::PaletteIndex::Max)
			{
				auto pal = GetPalette();
				pal[(int)id] = (ImU32)ct::ivariant(strValue).getU();
				ApplyPalette(pal);
			}
		}
	}
}

///////////////////////////////////////////////////////
//// PRIVVATE /////////////////////////////////////////
///////////////////////////////////////////////////////

const char* ImGuiThemeHelper::Get_ImGui_NameFromCol(ImGuiCol idx)
{
	switch (idx)
	{
	case ImGuiCol_Text: return "ImGuiCol_Text";
	case ImGuiCol_TextDisabled: return "ImGuiCol_TextDisabled";
	case ImGuiCol_WindowBg: return "ImGuiCol_WindowBg";
	case ImGuiCol_ChildBg: return "ImGuiCol_ChildBg";
	case ImGuiCol_PopupBg: return "ImGuiCol_PopupBg";
	case ImGuiCol_Border: return "ImGuiCol_Border";
	case ImGuiCol_BorderShadow: return "ImGuiCol_BorderShadow";
	case ImGuiCol_FrameBg: return "ImGuiCol_FrameBg";
	case ImGuiCol_FrameBgHovered: return "ImGuiCol_FrameBgHovered";
	case ImGuiCol_FrameBgActive: return "ImGuiCol_FrameBgActive";
	case ImGuiCol_TitleBg: return "ImGuiCol_TitleBg";
	case ImGuiCol_TitleBgActive: return "ImGuiCol_TitleBgActive";
	case ImGuiCol_TitleBgCollapsed: return "ImGuiCol_TitleBgCollapsed";
	case ImGuiCol_MenuBarBg: return "ImGuiCol_MenuBarBg";
	case ImGuiCol_ScrollbarBg: return "ImGuiCol_ScrollbarBg";
	case ImGuiCol_ScrollbarGrab: return "ImGuiCol_ScrollbarGrab";
	case ImGuiCol_ScrollbarGrabHovered: return "ImGuiCol_ScrollbarGrabHovered";
	case ImGuiCol_ScrollbarGrabActive: return "ImGuiCol_ScrollbarGrabActive";
	case ImGuiCol_CheckMark: return "ImGuiCol_CheckMark";
	case ImGuiCol_SliderGrab: return "ImGuiCol_SliderGrab";
	case ImGuiCol_SliderGrabActive: return "ImGuiCol_SliderGrabActive";
	case ImGuiCol_Button: return "ImGuiCol_Button";
	case ImGuiCol_ButtonHovered: return "ImGuiCol_ButtonHovered";
	case ImGuiCol_ButtonActive: return "ImGuiCol_ButtonActive";
	case ImGuiCol_Header: return "ImGuiCol_Header";
	case ImGuiCol_HeaderHovered: return "ImGuiCol_HeaderHovered";
	case ImGuiCol_HeaderActive: return "ImGuiCol_HeaderActive";
	case ImGuiCol_Separator: return "ImGuiCol_Separator";
	case ImGuiCol_SeparatorHovered: return "ImGuiCol_SeparatorHovered";
	case ImGuiCol_SeparatorActive: return "ImGuiCol_SeparatorActive";
	case ImGuiCol_ResizeGrip: return "ImGuiCol_ResizeGrip";
	case ImGuiCol_ResizeGripHovered: return "ImGuiCol_ResizeGripHovered";
	case ImGuiCol_ResizeGripActive: return "ImGuiCol_ResizeGripActive";
	case ImGuiCol_Tab: return "ImGuiCol_Tab";
	case ImGuiCol_TabHovered: return "ImGuiCol_TabHovered";
	case ImGuiCol_TabActive: return "ImGuiCol_TabActive";
	case ImGuiCol_TabUnfocused: return "ImGuiCol_TabUnfocused";
	case ImGuiCol_TabUnfocusedActive: return "ImGuiCol_TabUnfocusedActive";
	case ImGuiCol_DockingPreview: return "ImGuiCol_DockingPreview";
	case ImGuiCol_DockingEmptyBg: return "ImGuiCol_DockingEmptyBg";
	case ImGuiCol_PlotLines: return "ImGuiCol_PlotLines";
	case ImGuiCol_PlotLinesHovered: return "ImGuiCol_PlotLinesHovered";
	case ImGuiCol_PlotHistogram: return "ImGuiCol_PlotHistogram";
	case ImGuiCol_PlotHistogramHovered: return "ImGuiCol_PlotHistogramHovered";
	case ImGuiCol_TextSelectedBg: return "ImGuiCol_TextSelectedBg";
	case ImGuiCol_DragDropTarget: return "ImGuiCol_DragDropTarget";
	case ImGuiCol_NavHighlight: return "ImGuiCol_NavHighlight";
	case ImGuiCol_NavWindowingHighlight: return "ImGuiCol_NavWindowingHighlight";
	case ImGuiCol_NavWindowingDimBg: return "ImGuiCol_NavWindowingDimBg";
	case ImGuiCol_ModalWindowDimBg: return "ImGuiCol_ModalWindowDimBg";
	//  ImGui Tables
	case ImGuiCol_TableHeaderBg: return "ImGuiCol_TableHeaderBg";
	case ImGuiCol_TableBorderStrong:  return "ImGuiCol_TableBorderStrong";
	case ImGuiCol_TableBorderLight:  return "ImGuiCol_TableBorderLight";
	case ImGuiCol_TableRowBg:  return "ImGuiCol_TableRowBg";
	case ImGuiCol_TableRowBgAlt : return "ImGuiCol_TableRowBgAlt";
	//
	default:;
	}
	return "ImGuiCol_Unknow";
}

int ImGuiThemeHelper::Get_ImGui_ColFromName(const std::string& vName)
{
	if (vName == "ImGuiCol_Text") return ImGuiCol_Text;
	else if (vName == "ImGuiCol_TextDisabled") return ImGuiCol_TextDisabled;
	else if (vName == "ImGuiCol_WindowBg") return ImGuiCol_WindowBg;
	else if (vName == "ImGuiCol_ChildBg") return ImGuiCol_ChildBg;
	else if (vName == "ImGuiCol_PopupBg") return ImGuiCol_PopupBg;
	else if (vName == "ImGuiCol_Border") return ImGuiCol_Border;
	else if (vName == "ImGuiCol_BorderShadow") return ImGuiCol_BorderShadow;
	else if (vName == "ImGuiCol_FrameBg") return ImGuiCol_FrameBg;
	else if (vName == "ImGuiCol_FrameBgHovered") return ImGuiCol_FrameBgHovered;
	else if (vName == "ImGuiCol_FrameBgActive") return ImGuiCol_FrameBgActive;
	else if (vName == "ImGuiCol_TitleBg") return ImGuiCol_TitleBg;
	else if (vName == "ImGuiCol_TitleBgActive") return ImGuiCol_TitleBgActive;
	else if (vName == "ImGuiCol_TitleBgCollapsed") return ImGuiCol_TitleBgCollapsed;
	else if (vName == "ImGuiCol_MenuBarBg") return ImGuiCol_MenuBarBg;
	else if (vName == "ImGuiCol_ScrollbarBg") return ImGuiCol_ScrollbarBg;
	else if (vName == "ImGuiCol_ScrollbarGrab") return ImGuiCol_ScrollbarGrab;
	else if (vName == "ImGuiCol_ScrollbarGrabHovered") return ImGuiCol_ScrollbarGrabHovered;
	else if (vName == "ImGuiCol_ScrollbarGrabActive") return ImGuiCol_ScrollbarGrabActive;
	else if (vName == "ImGuiCol_CheckMark") return ImGuiCol_CheckMark;
	else if (vName == "ImGuiCol_SliderGrab") return ImGuiCol_SliderGrab;
	else if (vName == "ImGuiCol_SliderGrabActive") return ImGuiCol_SliderGrabActive;
	else if (vName == "ImGuiCol_Button") return ImGuiCol_Button;
	else if (vName == "ImGuiCol_ButtonHovered") return ImGuiCol_ButtonHovered;
	else if (vName == "ImGuiCol_ButtonActive") return ImGuiCol_ButtonActive;
	else if (vName == "ImGuiCol_Header") return ImGuiCol_Header;
	else if (vName == "ImGuiCol_HeaderHovered") return ImGuiCol_HeaderHovered;
	else if (vName == "ImGuiCol_HeaderActive") return ImGuiCol_HeaderActive;
	else if (vName == "ImGuiCol_Separator") return ImGuiCol_Separator;
	else if (vName == "ImGuiCol_SeparatorHovered") return ImGuiCol_SeparatorHovered;
	else if (vName == "ImGuiCol_SeparatorActive") return ImGuiCol_SeparatorActive;
	else if (vName == "ImGuiCol_ResizeGrip") return ImGuiCol_ResizeGrip;
	else if (vName == "ImGuiCol_ResizeGripHovered") return ImGuiCol_ResizeGripHovered;
	else if (vName == "ImGuiCol_ResizeGripActive") return ImGuiCol_ResizeGripActive;
	else if (vName == "ImGuiCol_Tab") return ImGuiCol_Tab;
	else if (vName == "ImGuiCol_TabHovered") return ImGuiCol_TabHovered;
	else if (vName == "ImGuiCol_TabActive") return ImGuiCol_TabActive;
	else if (vName == "ImGuiCol_TabUnfocused") return ImGuiCol_TabUnfocused;
	else if (vName == "ImGuiCol_TabUnfocusedActive") return ImGuiCol_TabUnfocusedActive;
	else if (vName == "ImGuiCol_DockingPreview") return ImGuiCol_DockingPreview;
	else if (vName == "ImGuiCol_DockingEmptyBg") return ImGuiCol_DockingEmptyBg;
	else if (vName == "ImGuiCol_PlotLines") return ImGuiCol_PlotLines;
	else if (vName == "ImGuiCol_PlotLinesHovered") return ImGuiCol_PlotLinesHovered;
	else if (vName == "ImGuiCol_PlotHistogram") return ImGuiCol_PlotHistogram;
	else if (vName == "ImGuiCol_PlotHistogramHovered") return ImGuiCol_PlotHistogramHovered;
	else if (vName == "ImGuiCol_TextSelectedBg") return ImGuiCol_TextSelectedBg;
	else if (vName == "ImGuiCol_DragDropTarget") return ImGuiCol_DragDropTarget;
	else if (vName == "ImGuiCol_NavHighlight") return ImGuiCol_NavHighlight;
	else if (vName == "ImGuiCol_NavWindowingHighlight") return ImGuiCol_NavWindowingHighlight;
	else if (vName == "ImGuiCol_NavWindowingDimBg") return ImGuiCol_NavWindowingDimBg;
	else if (vName == "ImGuiCol_ModalWindowDimBg") return ImGuiCol_ModalWindowDimBg;

	//  imGui Tables
	else if (vName == "ImGuiCol_TableHeaderBg") return ImGuiCol_TableHeaderBg;
	else if (vName == "ImGuiCol_TableBorderStrong") return ImGuiCol_TableBorderStrong;
	else if (vName == "ImGuiCol_TableBorderLight") return ImGuiCol_TableBorderLight;
	else if (vName == "ImGuiCol_TableRowBg") return ImGuiCol_TableRowBg;
	else if (vName == "ImGuiCol_TableRowBgAlt") return ImGuiCol_TableRowBgAlt;

	return -1;
}

const char* ImGuiThemeHelper::Get_ImGuiColorTextEditPalette_NameFromCol(TextEditor::PaletteIndex idx)
{
	switch (idx)
	{
	case TextEditor::PaletteIndex::Default: return "Default";
	case TextEditor::PaletteIndex::Keyword: return "Keyword";
	case TextEditor::PaletteIndex::Number: return "Number";
	case TextEditor::PaletteIndex::String: return "String";
	case TextEditor::PaletteIndex::CharLiteral: return "CharLiteral";
	case TextEditor::PaletteIndex::Punctuation: return "Punctuation";
	case TextEditor::PaletteIndex::Preprocessor: return "Preprocessor";
	case TextEditor::PaletteIndex::Identifier: return "Identifier";
	case TextEditor::PaletteIndex::KnownIdentifier: return "KnownIdentifier";
	case TextEditor::PaletteIndex::PreprocIdentifier: return "PreprocIdentifier";
	case TextEditor::PaletteIndex::Comment: return "Comment";
	case TextEditor::PaletteIndex::MultiLineComment: return "MultiLineComment";
	case TextEditor::PaletteIndex::Background: return "Background";
	case TextEditor::PaletteIndex::Cursor: return "Cursor";
	case TextEditor::PaletteIndex::Selection: return "Selection";
	case TextEditor::PaletteIndex::ErrorMarker: return "ErrorMarker";
	case TextEditor::PaletteIndex::Breakpoint: return "Breakpoint";
	case TextEditor::PaletteIndex::LineNumber: return "LineNumber";
	case TextEditor::PaletteIndex::CurrentLineFill: return "CurrentLineFill";
	case TextEditor::PaletteIndex::CurrentLineFillInactive: return "CurrentLineFillInactive";
	case TextEditor::PaletteIndex::CurrentLineEdge: return "CurrentLineEdge";
	//
	default:;
	}
	return "Unknow";
}

TextEditor::PaletteIndex ImGuiThemeHelper::Get_ImGuiColorTextEditPalette_ColFromName(const std::string& vName)
{
	if (vName == "Default") return TextEditor::PaletteIndex::Default;
	if (vName == "Keyword") return TextEditor::PaletteIndex::Keyword;
	if (vName == "Number") return TextEditor::PaletteIndex::Number;
	if (vName == "String") return TextEditor::PaletteIndex::String;
	if (vName == "CharLiteral") return TextEditor::PaletteIndex::CharLiteral;
	if (vName == "Punctuation") return TextEditor::PaletteIndex::Punctuation;
	if (vName == "Preprocessor") return TextEditor::PaletteIndex::Preprocessor;
	if (vName == "Identifier") return TextEditor::PaletteIndex::Identifier;
	if (vName == "KnownIdentifier") return TextEditor::PaletteIndex::KnownIdentifier;
	if (vName == "PreprocIdentifier") return TextEditor::PaletteIndex::PreprocIdentifier;
	if (vName == "Comment") return TextEditor::PaletteIndex::Comment;
	if (vName == "MultiLineComment") return TextEditor::PaletteIndex::MultiLineComment;
	if (vName == "Background") return TextEditor::PaletteIndex::Background;
	if (vName == "Cursor") return TextEditor::PaletteIndex::Cursor;
	if (vName == "Selection") return TextEditor::PaletteIndex::Selection;
	if (vName == "ErrorMarker") return TextEditor::PaletteIndex::ErrorMarker;
	if (vName == "Breakpoint") return TextEditor::PaletteIndex::Breakpoint;
	if (vName == "LineNumber") return TextEditor::PaletteIndex::LineNumber;
	if (vName == "CurrentLineFill") return TextEditor::PaletteIndex::CurrentLineFill;
	if (vName == "CurrentLineFillInactive") return TextEditor::PaletteIndex::CurrentLineFillInactive;
	if (vName == "CurrentLineEdge") return TextEditor::PaletteIndex::CurrentLineEdge;

	return TextEditor::PaletteIndex::Max;
}

enum class PaletteIndex
{
	Default,
	Keyword,
	Number,
	String,
	CharLiteral,
	Punctuation,
	Preprocessor,
	Identifier,
	KnownIdentifier,
	PreprocIdentifier,
	Comment,
	MultiLineComment,
	Background,
	Cursor,
	Selection,
	ErrorMarker,
	Breakpoint,
	LineNumber,
	CurrentLineFill,
	CurrentLineFillInactive,
	CurrentLineEdge,
	Max
};