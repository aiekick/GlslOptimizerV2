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
#include "ProjectFile.h"

#include "Helper/Messaging.h"

#include <FileHelper.h>

ProjectFile::ProjectFile() = default;

ProjectFile::ProjectFile(const std::string& vFilePathName)
{
	m_ProjectFilePathName = FileHelper::Instance()->SimplifyFilePath(vFilePathName);
	auto ps = FileHelper::Instance()->ParsePathFileName(m_ProjectFilePathName);
	if (ps.isOk)
	{
		m_ProjectFilePath = ps.path;
	}
}

ProjectFile::~ProjectFile() = default;

void ProjectFile::Clear()
{
	m_ProjectFilePathName.clear();
	m_ProjectFilePath.clear();
	m_IsLoaded = false;
	m_IsThereAnyNotSavedChanged = false;
	
	Messaging::Instance()->Clear();
}

void ProjectFile::New()
{
	Clear();
	m_IsLoaded = true;
	m_NeverSaved = true;
}

void ProjectFile::New(const std::string& vFilePathName)
{
	Clear();
	m_ProjectFilePathName = FileHelper::Instance()->SimplifyFilePath(vFilePathName);
	auto ps = FileHelper::Instance()->ParsePathFileName(m_ProjectFilePathName);
	if (ps.isOk)
	{
		m_ProjectFilePath = ps.path;
	}
	m_IsLoaded = true;
	SetProjectChange(false);
}

bool ProjectFile::Load()
{
	return LoadAs(m_ProjectFilePathName);
}

bool ProjectFile::LoadAs(const std::string& vFilePathName)
{
	Clear();
	std::string filePathName = FileHelper::Instance()->SimplifyFilePath(vFilePathName);
	if (LoadConfigFile(filePathName))
	{
		m_ProjectFilePathName = filePathName;
		auto ps = FileHelper::Instance()->ParsePathFileName(m_ProjectFilePathName);
		if (ps.isOk)
		{
			m_ProjectFilePath = ps.path;
		}
		m_IsLoaded = true;
		SetProjectChange(false);
	}
	else
	{
		Clear();
	}

	return m_IsLoaded;
}

bool ProjectFile::Save()
{
	if (m_NeverSaved) 
		return false;

	if (SaveConfigFile(m_ProjectFilePathName))
	{
		SetProjectChange(false);
		return true;
	}

	return false;
}

bool ProjectFile::SaveAs(const std::string& vFilePathName)
{
	std::string filePathName = FileHelper::Instance()->SimplifyFilePath(vFilePathName);
	auto ps = FileHelper::Instance()->ParsePathFileName(filePathName);
	if (ps.isOk)
	{
		m_ProjectFilePathName = FileHelper::Instance()->ComposePath(ps.path, ps.name, "ifs");
		m_ProjectFilePath = ps.path;
		m_NeverSaved = false;
		return Save();
	}
	return false;
}

bool ProjectFile::IsLoaded() const
{
	return m_IsLoaded;
}

bool ProjectFile::IsThereAnyNotSavedChanged() const
{
	return m_IsThereAnyNotSavedChanged;
}

void ProjectFile::SetProjectChange(bool vChange)
{
	m_IsThereAnyNotSavedChanged = vChange;
}

std::string ProjectFile::GetAbsolutePath(const std::string& vFilePathName) const
{
	std::string res = vFilePathName;

	if (!vFilePathName.empty())
	{
		if (!FileHelper::Instance()->IsAbsolutePath(vFilePathName)) // relative
		{
			res = FileHelper::Instance()->SimplifyFilePath(
				m_ProjectFilePath + FileHelper::Instance()->m_SlashType + vFilePathName);
		}
	}

	return res;
}

std::string ProjectFile::GetRelativePath(const std::string& vFilePathName) const
{
	std::string res = vFilePathName;

	if (!vFilePathName.empty())
	{
		res = FileHelper::Instance()->GetRelativePathToPath(vFilePathName, m_ProjectFilePath);
	}

	return res;
}

std::string ProjectFile::getXml(const std::string& vOffset)
{
	std::string str;

	str += vOffset + "<project>\n";

	str += vOffset + "</project>\n";

	return str;
}

void ProjectFile::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent)
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

	if (strName == "project")
	{
		for (tinyxml2::XMLElement* child = vElem->FirstChildElement(); child != nullptr; child = child->NextSiblingElement())
		{
			RecursParsingConfig(child->ToElement(), vElem);
		}
	}

	if (strParentName == "project")
	{
		
	}
}
