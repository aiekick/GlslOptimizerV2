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

#include <imgui/imgui.h>

#include <stdint.h>
#include <string>
#include <map>

#include "ImGuiColorTextEdit/TextEditor.h"
#include "ctools/GLVersionChecker.h"
#include "src/code/GlslConvert.h"

class ProjectFile;
class OptimizerPane
{
private:
	enum OptimizerPaneFlags
	{
		OPT_PANE_NONE = 0,
		OPT_PANE_OPTIMIZATION = (1 << 0),
		OPT_PANE_COMPILER = (1 << 1)
	} m_OptimizerPaneFlags = OPT_PANE_OPTIMIZATION;

private:
	OpenGlVersionStruct m_Current_OpenGlVersionStruct;

public:
	void Init();
	int DrawPane(ProjectFile *vProjectFile, int vWidgetId);
	void ChangeGLSLVersionInCode(const std::string& vNewVersionCode = "");

private:
	void Generate(ProjectFile *vProjectFile);
	bool DrawOptimizationFlags(ProjectFile *vProjectFile, ImVec2 vSize);
	bool DrawCompilerFlags(ProjectFile *vProjectFile, ImVec2 vSize);
	bool DrawInstructionToLowerFlags(ProjectFile *vProjectFile, ImVec2 vSize);
	
public: // singleton
	static OptimizerPane *Instance()
	{
		static OptimizerPane *_instance = new OptimizerPane();
		return _instance;
	}

protected:
	OptimizerPane(); // Prevent construction
	OptimizerPane(const OptimizerPane&) {}; // Prevent construction by copying
	OptimizerPane& operator =(const OptimizerPane&) { return *this; }; // Prevent assignment
	~OptimizerPane(); // Prevent unwanted destruction};
};
