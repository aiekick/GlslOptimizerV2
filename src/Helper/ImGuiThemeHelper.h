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
#pragma once

#include <ImGuiFileDialog/ImGuiFileDialog/ImGuiFileDialog.h>
#include <ConfigAbstract.h>
#include <imgui.h>
#include <string>
#include <map>
#include "ImGuiColorTextEdit/TextEditor.h"

class ImGuiThemeHelper : public conf::ConfigAbstract
{
public:
	ImVec4 goodColor = ImVec4(0.2f, 0.8f, 0.2f, 1.0f);
	ImVec4 badColor = ImVec4(0.8f, 0.2f, 0.2f, 1.0f);

private:
	std::map<std::string, igfd::FilterInfosStruct> m_FileTypeInfos;
	bool m_ShowImGuiStyle = false;
	bool m_ShowImGuiEditorStyle = false;

public:
	void Draw();
	void DrawMenu();
	void DrawImGuiStyleEditor(bool *vOpen);
	void DrawImGuiColorTextEditStyleEditor(bool *vOpen);
	
public:
	std::string getXml(const std::string& vOffset) override;
	void setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent) override;

public:
	void ApplyStyleColorsClassic(ImGuiStyle* dst = nullptr);
	void ApplyStyleColorsDark(ImGuiStyle* dst = nullptr);
	void ApplyStyleColorsLight(ImGuiStyle* dst = nullptr);
	void ApplyPalette(TextEditor::Palette vPalette);
	TextEditor::Palette GetPalette();
	void ApplyFileTypeColors();
	
private:
	// ImGui Styles
	static const char* Get_ImGui_NameFromCol(ImGuiCol idx);
	static int Get_ImGui_ColFromName(const std::string& vName);
	// ImGuiColorTextEdit Palette
	static const char* Get_ImGuiColorTextEditPalette_NameFromCol(TextEditor::PaletteIndex idx);
	static TextEditor::PaletteIndex Get_ImGuiColorTextEditPalette_ColFromName(const std::string& vName);

public: // singleton
	static ImGuiThemeHelper *Instance()
	{
		static auto *_instance = new ImGuiThemeHelper();
		return _instance;
	}

protected:
	ImGuiThemeHelper(); // Prevent construction
	ImGuiThemeHelper(const ImGuiThemeHelper&) {}; // Prevent construction by copying
	ImGuiThemeHelper& operator =(const ImGuiThemeHelper&) { return *this; }; // Prevent assignment
	~ImGuiThemeHelper(); // Prevent unwanted destruction
};

