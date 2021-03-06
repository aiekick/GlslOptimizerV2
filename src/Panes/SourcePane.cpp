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

#include "SourcePane.h"

#include "MainFrame.h"

#include "Gui/GuiLayout.h"
#include "Gui/ImGuiWidgets.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include <cTools.h>
#include <FileHelper.h>

#include <cinttypes> // printf zu

static int SourcePane_WidgetId = 0;

SourcePane::SourcePane() = default;
SourcePane::~SourcePane() = default;

void SourcePane::Init()
{
	m_CodeEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::GLSL());
	m_CodeEditor.SetPalette(TextEditor::GetLightPalette());
}

///////////////////////////////////////////////////////////////////////////////////
//// IMGUI PANE ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

int SourcePane::DrawPane(ProjectFile *vProjectFile, int vWidgetId)
{
	SourcePane_WidgetId = vWidgetId;

	if (GuiLayout::m_Pane_Shown & PaneFlags::PANE_SOURCE)
	{
		if (ImGui::Begin<PaneFlags>(SOURCE_PANE,
			&GuiLayout::m_Pane_Shown, PaneFlags::PANE_SOURCE,
			//ImGuiWindowFlags_NoTitleBar |
			//ImGuiWindowFlags_MenuBar |
			//ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			//ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			if (vProjectFile &&  vProjectFile->IsLoaded())
			{
				m_CodeEditor.Render("Source", ImVec2(-1, -1), false);
				if (m_CodeEditor.IsTextChanged())
				{
					vProjectFile->SetProjectChange();
				}
			}
		}

		ImGui::End();
	}

	return SourcePane_WidgetId;
}

std::string SourcePane::GetCode()
{
	return m_CodeEditor.GetText();
}

void SourcePane::SetCode(std::string vCode)
{
	m_CodeEditor.SetText(vCode);
}

