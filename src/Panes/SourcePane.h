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

#ifdef _DEBUG

#include <imgui/imgui.h>

#include <stdint.h>
#include <string>
#include <map>

class ProjectFile;
class SourcePane
{
public:
	int DrawPane(ProjectFile *vProjectFile, int vWidgetId);

public: // singleton
	static SourcePane *Instance()
	{
		static SourcePane *_instance = new SourcePane();
		return _instance;
	}

protected:
	SourcePane(); // Prevent construction
	SourcePane(const SourcePane&) {}; // Prevent construction by copying
	SourcePane& operator =(const SourcePane&) { return *this; }; // Prevent assignment
	~SourcePane(); // Prevent unwanted destruction};
};

#endif