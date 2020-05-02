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

#include "TargetPane.h"

#include "MainFrame.h"

#include "Gui/GuiLayout.h"
#include "Gui/ImGuiWidgets.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include <cTools.h>
#include <FileHelper.h>

#include <cinttypes> // printf zu

static int TargetPane_WidgetId = 0;

TargetPane::TargetPane() = default;
TargetPane::~TargetPane() = default;

void TargetPane::Init()
{
	m_CodeEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::GLSL());
	m_CodeEditor.SetPalette(TextEditor::GetLightPalette());
}

///////////////////////////////////////////////////////////////////////////////////
//// IMGUI PANE ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

int TargetPane::DrawPane(ProjectFile *vProjectFile, int vWidgetId)
{
	TargetPane_WidgetId = vWidgetId;

	if (GuiLayout::m_Pane_Shown & PaneFlags::PANE_TARGET)
	{
		if (ImGui::Begin<PaneFlags>(TARGET_PANE,
			&GuiLayout::m_Pane_Shown, PaneFlags::PANE_TARGET,
			//ImGuiWindowFlags_NoTitleBar |
			//ImGuiWindowFlags_MenuBar |
			//ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			//ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			if (vProjectFile &&  vProjectFile->IsLoaded())
			{
				m_CodeEditor.Render("Target", ImVec2(-1, -1), false);
			}
		}

		ImGui::End();
	}

	return TargetPane_WidgetId;
}

std::string TargetPane::GetCode()
{
	return m_CodeEditor.GetText();
}

void TargetPane::SetCode(std::string vCode)
{
	m_CodeEditor.SetText(vCode);
}


