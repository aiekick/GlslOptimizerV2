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
#include "Panes/SourcePane.h"
#include "Panes/TargetPane.h"
#include "Panes/OptimizerPane.h"

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

	SourcePane::Instance()->GetEditor()->Delete();
	SourcePane::Instance()->GetEditor()->SetText("");
	TargetPane::Instance()->GetEditor()->Delete();
	TargetPane::Instance()->GetEditor()->SetText("");

	m_ShaderStage = GlslConvert::ShaderStage::MESA_SHADER_FRAGMENT;
	m_ApiTarget = GlslConvert::ApiTarget::API_OPENGL_CORE;
	m_LanguageTarget = GlslConvert::LanguageTarget::LANGUAGE_TARGET_GLSL;
	m_OptimizationStruct = GlslConvert::OptimizationStruct();

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
	if (!vFilePathName.empty())
	{
		m_ProjectFilePathName = FileHelper::Instance()->SimplifyFilePath(vFilePathName);
		auto ps = FileHelper::Instance()->ParsePathFileName(m_ProjectFilePathName);
		if (ps.isOk)
		{
			m_ProjectFilePath = ps.path;
			OptimizerPane::Instance()->ChangeGLSLVersionInCode();
		}
		SetProjectChange(false);
	}
	else
	{
		m_NeverSaved = true;
	}
	m_IsLoaded = true;
}

bool ProjectFile::Load()
{
	return LoadAs(m_ProjectFilePathName);
}

bool ProjectFile::LoadAs(const std::string& vFilePathName)
{
	Clear();

	std::string filePathName = FileHelper::Instance()->SimplifyFilePath(vFilePathName);
	if (FileHelper::Instance()->IsFileExist(filePathName))
	{
		std::string code = FileHelper::Instance()->LoadFileToString(filePathName);
		if (!code.empty())
		{
			SourcePane::Instance()->SetCode(code);
			m_ProjectFilePathName = filePathName;

			auto ps = FileHelper::Instance()->ParsePathFileName(filePathName);
			if (ps.isOk)
			{
				std::string configFile = ps.GetFilePathWithNameExt(ps.name + "_" + ps.ext, ".conf");
				if (FileHelper::Instance()->IsFileExist(configFile))
				{
					if (LoadConfigFile(configFile))
					{
						auto ps = FileHelper::Instance()->ParsePathFileName(m_ProjectFilePathName);
						if (ps.isOk)
						{
							m_ProjectFilePath = ps.path;
						}
					}
				}
			}

			m_IsLoaded = true;
			SetProjectChange(false);
		}
	}

	return m_IsLoaded;
}

bool ProjectFile::Save()
{
	return SaveAs(m_ProjectFilePathName);
}

bool ProjectFile::SaveAs(const std::string& vFilePathName)
{
	bool res = false;

	if (vFilePathName.empty())
		return res;

	std::string filePathName = FileHelper::Instance()->SimplifyFilePath(vFilePathName);
	auto ps = FileHelper::Instance()->ParsePathFileName(filePathName);
	if (ps.isOk)
	{
		m_ProjectFilePathName = filePathName;
		m_ProjectFilePath = ps.path;

		std::string code = SourcePane::Instance()->GetCode();
		if (!code.empty())
		{
			FileHelper::Instance()->SaveStringToFile(m_ProjectFilePathName, code);

			std::string configFile = ps.GetFilePathWithNameExt(ps.name + "_" + ps.ext, ".conf");
			if (SaveConfigFile(configFile))
			{
				SetProjectChange(false);

				res = true;
			}
		}
	}
	
	return res;
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

	std::string offset = vOffset + "\t";

	str += offset + "<stage>" + ct::toStr(m_ShaderStage) + "</stage>\n";
	str += offset + "<api_target>" + ct::toStr(m_ApiTarget) + "</api_target>\n";
	str += offset + "<language_target>" + ct::toStr(m_LanguageTarget) + "</language_target>\n";
	
	str += getXml_From_OptimizationStruct(offset);

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
		if (strName == "stage") m_ShaderStage = (GlslConvert::ShaderStage)ct::ivariant(strValue).getU();
		if (strName == "api_target") m_ApiTarget = (GlslConvert::ApiTarget)ct::ivariant(strValue).getU();
		if (strName == "language_target") m_LanguageTarget = (GlslConvert::LanguageTarget)ct::ivariant(strValue).getU();
	}

	setOptimizationStruct_From_Xml(vElem, vParent);
}

std::string ProjectFile::getXml_From_OptimizationStruct(const std::string& vOffset)
{
	std::string str;
	
	str += vOffset + "<optimization>\n";

	std::string offset = vOffset + "\t";

	str += offset + "<compiler_flags>" + ct::toStr(m_OptimizationStruct.compilerFlags) + "</compiler_flags>\n";
	str += offset + "<control_flags>" + ct::toStr(m_OptimizationStruct.controlFlags) + "</control_flags>\n";
	str += offset + "<optimization_flags>" + ct::toStr(m_OptimizationStruct.optimizationFlags) + "</optimization_flags>\n";
	str += offset + "<optimization_flags_bis>" + ct::toStr(m_OptimizationStruct.optimizationFlags_Bis) + "</optimization_flags_bis>\n";
	str += offset + "<instructiontolower_flags>" + ct::toStr(m_OptimizationStruct.instructionToLowerFlags) + "</instructiontolower_flags>\n";

	str += offset + "<algebraic_native_integers>" + ct::toStr(m_OptimizationStruct.algebraicOptions.native_integers) + "</algebraic_native_integers>\n";

	str += offset + "<lower_jump_pull_out_jumps>" + ct::toStr(m_OptimizationStruct.lowerJumpsOptions.pull_out_jumps) + "</lower_jump_pull_out_jumps>\n";
	str += offset + "<lower_jump_lower_sub_return>" + ct::toStr(m_OptimizationStruct.lowerJumpsOptions.lower_sub_return) + "</lower_jump_lower_sub_return>\n";
	str += offset + "<lower_jump_lower_main_return>" + ct::toStr(m_OptimizationStruct.lowerJumpsOptions.lower_main_return) + "</lower_jump_lower_main_return>\n";
	str += offset + "<lower_jump_lower_continue>" + ct::toStr(m_OptimizationStruct.lowerJumpsOptions.lower_continue) + "</lower_jump_lower_continue>\n";
	str += offset + "<lower_jump_lower_break>" + ct::toStr(m_OptimizationStruct.lowerJumpsOptions.lower_break) + "</lower_jump_lower_break>\n";

	str += offset + "<lower_if_to_cond_assign_max_depth>" + ct::toStr(m_OptimizationStruct.lowerIfToCondAssignOptions.max_depth) + "</lower_if_to_cond_assign_max_depth>\n";
	str += offset + "<lower_if_to_cond_assign_min_branch_cost>" + ct::toStr(m_OptimizationStruct.lowerIfToCondAssignOptions.min_branch_cost) + "</lower_if_to_cond_assign_min_branch_cost>\n";

	str += offset + "<lower_variable_index_to_cond_assign_lower_input>" + ct::toStr(m_OptimizationStruct.lowerVariableIndexToCondAssignOptions.lower_input) + "</lower_variable_index_to_cond_assign_lower_input>\n";
	str += offset + "<lower_variable_index_to_cond_assign_lower_output>" + ct::toStr(m_OptimizationStruct.lowerVariableIndexToCondAssignOptions.lower_output) + "</lower_variable_index_to_cond_assign_lower_output>\n";
	str += offset + "<lower_variable_index_to_cond_assign_lower_temp>" + ct::toStr(m_OptimizationStruct.lowerVariableIndexToCondAssignOptions.lower_temp) + "</lower_variable_index_to_cond_assign_lower_temp>\n";
	str += offset + "<lower_variable_index_to_cond_assign_lower_uniform>" + ct::toStr(m_OptimizationStruct.lowerVariableIndexToCondAssignOptions.lower_uniform) + "</lower_variable_index_to_cond_assign_lower_uniform>\n";

	str += offset + "<dead_code_keep_only_assigned_uniforms>" + ct::toStr(m_OptimizationStruct.deadCodeOptions.keep_only_assigned_uniforms) + "</dead_code_keep_only_assigned_uniforms>\n";

	str += offset + "<dead_function_entryFunc>" + m_OptimizationStruct.deadFunctionOptions.entryFunc + "</dead_function_entryFunc>\n";

	str += offset + "<lower_vector_insert_lower_nonconstant_index>" + ct::toStr(m_OptimizationStruct.lowerVectorInsertOptions.lower_nonconstant_index) + "</lower_vector_insert_lower_nonconstant_index>\n";
	
	str += offset + "<lower_quadop_vector_dont_lower_swz>" + ct::toStr(m_OptimizationStruct.lowerQuadopVector.dont_lower_swz) + "</lower_quadop_vector_dont_lower_swz>\n";

	str += offset + "<instruction_to_lower_max_if_depth>" + ct::toStr(m_OptimizationStruct.instructionToLower.MaxIfDepth) + "</instruction_to_lower_max_if_depth>\n";
	str += offset + "<instruction_to_lower_max_unroll_iterations>" + ct::toStr(m_OptimizationStruct.instructionToLower.MaxUnrollIterations) + "</instruction_to_lower_max_unroll_iterations>\n";

	str += vOffset + "</optimization>\n";

	return str;
}

void ProjectFile::setOptimizationStruct_From_Xml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent)
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

	if (strName == "optimization" && strParentName == "project")
	{
		for (tinyxml2::XMLElement* child = vElem->FirstChildElement(); child != nullptr; child = child->NextSiblingElement())
		{
			RecursParsingConfig(child->ToElement(), vElem);
		}
	}

	if (strParentName == "optimization")
	{
		if (strName == "compiler_flags") m_OptimizationStruct.compilerFlags = (GlslConvert::CompilerFlags)ct::ivariant(strValue).getI();
		if (strName == "control_flags") m_OptimizationStruct.controlFlags = (GlslConvert::ControlFlags)ct::ivariant(strValue).getI();
		if (strName == "optimization_flags") m_OptimizationStruct.optimizationFlags = (GlslConvert::OptimizationFlags)ct::ivariant(strValue).getI();
		if (strName == "optimization_flags_bis") m_OptimizationStruct.optimizationFlags_Bis = (GlslConvert::OptimizationFlags_Bis)ct::ivariant(strValue).getI();
		if (strName == "instructiontolower_flags") m_OptimizationStruct.instructionToLowerFlags = (GlslConvert::InstructionToLowerFlags)ct::ivariant(strValue).getI();

		if (strName == "algebraic_native_integers") m_OptimizationStruct.algebraicOptions.native_integers = ct::ivariant(strValue).getB();

		if (strName == "lower_jump_pull_out_jumps") m_OptimizationStruct.lowerJumpsOptions.pull_out_jumps = ct::ivariant(strValue).getB();
		if (strName == "lower_jump_lower_sub_return") m_OptimizationStruct.lowerJumpsOptions.lower_sub_return = ct::ivariant(strValue).getB();
		if (strName == "lower_jump_lower_main_return") m_OptimizationStruct.lowerJumpsOptions.lower_main_return = ct::ivariant(strValue).getB();
		if (strName == "lower_jump_lower_continue") m_OptimizationStruct.lowerJumpsOptions.lower_continue = ct::ivariant(strValue).getB();
		if (strName == "lower_jump_lower_break") m_OptimizationStruct.lowerJumpsOptions.lower_break = ct::ivariant(strValue).getB();

		if (strName == "lower_if_to_cond_assign_max_depth") m_OptimizationStruct.lowerIfToCondAssignOptions.max_depth = ct::ivariant(strValue).getI();
		if (strName == "lower_if_to_cond_assign_min_branch_cost") m_OptimizationStruct.lowerIfToCondAssignOptions.min_branch_cost = ct::ivariant(strValue).getI();

		if (strName == "lower_variable_index_to_cond_assign_lower_input") m_OptimizationStruct.lowerVariableIndexToCondAssignOptions.lower_input = ct::ivariant(strValue).getB();
		if (strName == "lower_variable_index_to_cond_assign_lower_output") m_OptimizationStruct.lowerVariableIndexToCondAssignOptions.lower_output = ct::ivariant(strValue).getB();
		if (strName == "lower_variable_index_to_cond_assign_lower_temp") m_OptimizationStruct.lowerVariableIndexToCondAssignOptions.lower_temp = ct::ivariant(strValue).getB();
		if (strName == "lower_variable_index_to_cond_assign_lower_uniform") m_OptimizationStruct.lowerVariableIndexToCondAssignOptions.lower_uniform = ct::ivariant(strValue).getB();

		if (strName == "dead_code_keep_only_assigned_uniforms") m_OptimizationStruct.deadCodeOptions.keep_only_assigned_uniforms = ct::ivariant(strValue).getB();

		if (strName == "dead_function_entryFunc") m_OptimizationStruct.deadFunctionOptions.entryFunc = strValue;

		if (strName == "lower_vector_insert_lower_nonconstant_index") m_OptimizationStruct.lowerVectorInsertOptions.lower_nonconstant_index = ct::ivariant(strValue).getB();

		if (strName == "lower_quadop_vector_dont_lower_swz") m_OptimizationStruct.lowerQuadopVector.dont_lower_swz = ct::ivariant(strValue).getB();

		if (strName == "instruction_to_lower_max_if_depth") m_OptimizationStruct.instructionToLower.MaxIfDepth = ct::ivariant(strValue).getI();
		if (strName == "instruction_to_lower_max_unroll_iterations") m_OptimizationStruct.instructionToLower.MaxUnrollIterations = ct::ivariant(strValue).getI();
	}
}
