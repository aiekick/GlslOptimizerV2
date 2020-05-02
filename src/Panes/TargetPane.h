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

class ProjectFile;
class TargetPane
{
private:
	TextEditor m_CodeEditor;

public:
	void Init();
	int DrawPane(ProjectFile *vProjectFile, int vWidgetId);
	std::string GetCode();
	void SetCode(std::string vCode);
	
public: // singleton
	static TargetPane *Instance()
	{
		static TargetPane *_instance = new TargetPane();
		return _instance;
	}

protected:
	TargetPane(); // Prevent construction
	TargetPane(const TargetPane&) {}; // Prevent construction by copying
	TargetPane& operator =(const TargetPane&) { return *this; }; // Prevent assignment
	~TargetPane(); // Prevent unwanted destruction};
};
