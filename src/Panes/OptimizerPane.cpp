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

#include "OptimizerPane.h"

#include "MainFrame.h"

#include "Gui/GuiLayout.h"
#include "Gui/ImGuiWidgets.h"

#include "Panes/SourcePane.h"
#include "Panes/TargetPane.h"
#include "Project/ProjectFile.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include "ImGuiFileDialog/ImGuiFileDialog/ImGuiFileDialog.h"

#include <cTools.h>
#include <FileHelper.h>

#include <cinttypes> // printf zu

static int OptimizerPane_WidgetId = 0;

OptimizerPane::OptimizerPane() = default;
OptimizerPane::~OptimizerPane() = default;

void OptimizerPane::Init()
{
	auto v = GLVersionChecker::Instance()->GetOpenglVersionStruct(GLVersionChecker::Instance()->GetOpenglVersion());
	if (v)
		m_Current_OpenGlVersionStruct = *v;
}

///////////////////////////////////////////////////////////////////////////////////
//// IMGUI PANE ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

int OptimizerPane::DrawPane(ProjectFile *vProjectFile, int vWidgetId)
{
	OptimizerPane_WidgetId = vWidgetId;

	ImGui::SetPUSHID(OptimizerPane_WidgetId);

	bool change = false;

	if (GuiLayout::m_Pane_Shown & PaneFlags::PANE_OPTIMIZER)
	{
		if (ImGui::Begin<PaneFlags>(OPTIMIZER_PANE,
			&GuiLayout::m_Pane_Shown, PaneFlags::PANE_OPTIMIZER,
			//ImGuiWindowFlags_NoTitleBar |
			//ImGuiWindowFlags_MenuBar |
			//ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			//ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			if (vProjectFile &&  vProjectFile->IsLoaded())
			{
				ImGui::Header("Shader Stage");

				ImGui::Indent();
				{
					static int _shaderStage = (int)GlslConvert::ShaderStage::MESA_SHADER_VERTEX;
					_shaderStage = (int)vProjectFile->m_ShaderStage;
					if (ImGui::Combo("##shaderstage", &_shaderStage, "Vertex\0Tesselation Control\0Tesselation Evaluation\0Geometry\0Fragment\0\0"))
					{
						vProjectFile->m_ShaderStage = (GlslConvert::ShaderStage)_shaderStage;
						change = true;
					}
				}
				ImGui::Unindent();

				ImGui::Header("Opengl Version");

				ImGui::Indent();
				{
					ImGui::Text("Version code : %s", m_Current_OpenGlVersionStruct.DefineCode.c_str());
					if (ImGui::BeginCombo("##Opengl Version", m_Current_OpenGlVersionStruct.OpenglVersion.c_str()))
					{
						for (auto it = GLVersionChecker::Instance()->GetOpenglVersionMap()->begin();
							it != GLVersionChecker::Instance()->GetOpenglVersionMap()->end(); ++it)
						{
							if (it->second.supported)
							{
								if (ImGui::Selectable(it->second.OpenglVersion.c_str(),
									it->second.OpenglVersion == m_Current_OpenGlVersionStruct.OpenglVersion))
								{
									m_Current_OpenGlVersionStruct = it->second;
									ChangeGLSLVersionInCode(m_Current_OpenGlVersionStruct.DefineCode);
									vProjectFile->SetProjectChange();
								}
							}
						}
						change = true;
						ImGui::EndCombo();
					}

					if (m_Current_OpenGlVersionStruct.DefaultGlslVersionInt != 100 &&
						m_Current_OpenGlVersionStruct.DefaultGlslVersionInt != 300)
					{
						static int _targetType = (int)vProjectFile->m_ApiTarget;
						if (ImGui::Combo("##ApiTarget", &_targetType, "Compat\0Core\0\0"))
						{
							vProjectFile->m_ApiTarget = (GlslConvert::ApiTarget)_targetType;
							change = true;
						}
					}
				}
				ImGui::Unindent();

				ImGui::Header("Language Target");

				ImGui::Indent();
				{
					static int _conversionTarget = (int)vProjectFile->m_LanguageTarget;
					if (ImGui::Combo("##LanguageTarget", &_conversionTarget, "MESA AST\0MESA IR\0GLSL\0\0"))
					{
						vProjectFile->m_LanguageTarget = (GlslConvert::LanguageTarget)_conversionTarget;
						change = true;
					}
				}
				ImGui::Unindent();

				ImGui::Header("Generate");

				ImGui::Indent();
				{
					if (ImGui::Button("Optimize)"))
					{
						Generate(vProjectFile);
					}

					ImGui::Separator();

					ImGui::Text("Control :");
					ImGui::Indent();
					{
						change |= ImGui::CheckBoxBitWize<GlslConvert::ControlFlags>("Partial Shader", "Not Linked !\nso some optimizations will not been made",
							&vProjectFile->m_OptimizationStruct.controlFlags, GlslConvert::ControlFlags::CONTROL_DO_PARTIAL_SHADER, false);
						if (!(vProjectFile->m_OptimizationStruct.controlFlags & GlslConvert::ControlFlags::CONTROL_DO_PARTIAL_SHADER))
						{
							ImGui::Separator();
							change |= ImGui::CheckBoxBitWize<GlslConvert::ControlFlags>("Skip Preprocessor", "preprocessor directives and comments will be removed before link",
								&vProjectFile->m_OptimizationStruct.controlFlags, GlslConvert::ControlFlags::CONTROL_SKIP_PREPROCESSING, false);
						}
					}
					ImGui::Unindent();

					float y = ImGui::GetContentRegionAvail().y;
					change |= DrawOptimizationFlags(vProjectFile, ImVec2(-1, y));
					change |= DrawCompilerFlags(vProjectFile, ImVec2(-1, y));
				}
				ImGui::Unindent();
			}
		}

		ImGui::End();
	}

	if (change)
	{
		vProjectFile->SetProjectChange();
	}

	return OptimizerPane_WidgetId;
}

template<typename T>
inline bool DrawBitWizeToolBar(T *vContainer)
{
	bool change = false;
	ImGui::PushID(++OptimizerPane_WidgetId);
	if (ImGui::Button("Alls"))
	{
		if (vContainer)
			*vContainer = (T)(~(0)); // on inverse
		change = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("None"))
	{
		if (vContainer)
			*vContainer = (T)(0);
		change = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Inv"))
	{
		if (vContainer)
			*vContainer = (T)(~(*vContainer)); // on inverse
		change = true;
	}
	ImGui::PopID();
	return change;
}

template<typename T, typename U>
inline bool DrawBitWizeToolBar2(T *vContainer, U *vContainerBis)
{
	bool change = false;
	ImGui::PushID(++OptimizerPane_WidgetId);
	if (ImGui::Button("Alls"))
	{
		if (vContainer)
			*vContainer = (T)(~(0)); // on inverse
		if (vContainerBis)
			*vContainerBis = (U)(~(0)); // on inverse
		change = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("None"))
	{
		if (vContainer)
			*vContainer = (T)(0);
		if (vContainerBis)
			*vContainerBis = (U)(0);
		change = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Inv"))
	{
		if (vContainer)
			*vContainer = (T)(~(*vContainer)); // on inverse
		if (vContainerBis)
			*vContainerBis = (U)(~(*vContainerBis)); // on inverse
		change = true;
	}
	ImGui::PopID();
	return change;
}

bool OptimizerPane::DrawOptimizationFlags(ProjectFile *vProjectFile, ImVec2 vSize)
{
	bool change = false;

#define CHECK(STR, COM, FLAG, DEF) change |= ImGui::CheckBoxBitWize<GlslConvert::OptimizationFlags>(STR, COM, &vProjectFile->m_OptimizationStruct.optimizationFlags, GlslConvert::OptimizationFlags::FLAG, DEF)
#define CHECK_BIS(STR, COM, FLAG, DEF) change |= ImGui::CheckBoxBitWize<GlslConvert::OptimizationFlags_Bis>(STR, COM, &vProjectFile->m_OptimizationStruct.optimizationFlags_Bis, GlslConvert::OptimizationFlags_Bis::FLAG, DEF)
	float y = ImGui::GetCursorPosY();
	if (ImGui::ImGui_CollapsingHeader_BitWize_OneAtATime<OptimizerPaneFlags>("Optimization Settings", -1, 
		&m_OptimizerPaneFlags, OptimizerPaneFlags::OPT_PANE_OPTIMIZATION, false))
	{
		change |= DrawBitWizeToolBar2<GlslConvert::OptimizationFlags, GlslConvert::OptimizationFlags_Bis>(
			&vProjectFile->m_OptimizationStruct.optimizationFlags,
			&vProjectFile->m_OptimizationStruct.optimizationFlags_Bis);
		ImGui::Separator();
		vSize.x -= 10.0f;
		vSize.y -= (ImGui::GetCursorPosY() - y) * 2.0f;
		ImGui::BeginChild("##OptimizationSettings", vSize);
		ImGui::Indent();
		{
			CHECK("algebraic", 0, OPT_algebraic, true);
			if (vProjectFile->m_OptimizationStruct.optimizationFlags & GlslConvert::OptimizationFlags::OPT_algebraic)
			{
				ImGui::Indent();
				change |= ImGui::CheckBoxDefault("native_integers", &vProjectFile->m_OptimizationStruct.algebraicOptions.native_integers, false, 0);
				ImGui::Unindent();
			}
			ImGui::Separator();
			CHECK("common_optimization", 0, OPT_common_optimization, true);
			ImGui::Separator();
			CHECK("constant_folding", 0, OPT_constant_folding, true);
			ImGui::Separator();
			CHECK("constant_propagation", 0, OPT_constant_propagation, true);
			ImGui::Separator();
			CHECK("constant_variable", 0, OPT_constant_variable, true);
			ImGui::Separator();
			CHECK("constant_variable_unlinked", 0, OPT_constant_variable_unlinked, true);
			ImGui::Separator();
			CHECK("copy_propagation_elements", 0, OPT_copy_propagation_elements, true);
			ImGui::Separator();
			CHECK("dead_code", 0, OPT_dead_code, true);
			if (vProjectFile->m_OptimizationStruct.optimizationFlags & GlslConvert::OptimizationFlags::OPT_dead_code)
			{
				ImGui::Indent();
				GlslConvert::OptimizationStruct::DeadCodeOptions def;
				change |= ImGui::CheckBoxDefault("Keep only assigned uniforms",
					&vProjectFile->m_OptimizationStruct.deadCodeOptions.keep_only_assigned_uniforms,
					def.keep_only_assigned_uniforms, "true  => Keep only assigned uniforms\nfalse => Keep all uniforms");
				ImGui::Unindent();
			}
			ImGui::Separator();
			CHECK("dead_code_local", 0, OPT_dead_code_local, true);
			ImGui::Separator();
			CHECK("dead_code_unlinked", 0, OPT_dead_code_unlinked, true);
			ImGui::Separator();
			CHECK("dead_functions", 0, OPT_dead_functions, true);
			if (vProjectFile->m_OptimizationStruct.optimizationFlags & GlslConvert::OptimizationFlags::OPT_dead_functions)
			{
				ImGui::Indent();
				GlslConvert::OptimizationStruct::DeadFunctionOptions def;
				static char entryFuncBuffer[1024] = "\0";
				if (ImGui::Button("R"))
				{
					vProjectFile->m_OptimizationStruct.deadFunctionOptions.entryFunc = "main";
					change = true;
				}
				ImGui::SameLine();
				snprintf(entryFuncBuffer, 1023, vProjectFile->m_OptimizationStruct.deadFunctionOptions.entryFunc.c_str());
				ImGui::PushItemWidth(150.0f);
				if (ImGui::InputText("Entry Func", entryFuncBuffer, 1023))
				{
					vProjectFile->m_OptimizationStruct.deadFunctionOptions.entryFunc = entryFuncBuffer;
					change = true;
				}
				ImGui::PopItemWidth();
				ImGui::Unindent();
			}
			ImGui::Separator();
			CHECK("function_inlining", 0, OPT_function_inlining, true);
			ImGui::Separator();
			CHECK("if_simplification", 0, OPT_if_simplification, true);
			ImGui::Separator();
			CHECK("lower_discard", 0, OPT_lower_discard, true);
			ImGui::Separator();
			CHECK("lower_variable_index_to_cond_assign", 0, OPT_lower_variable_index_to_cond_assign, true);
			if (vProjectFile->m_OptimizationStruct.optimizationFlags & GlslConvert::OptimizationFlags::OPT_lower_variable_index_to_cond_assign)
			{
				ImGui::Indent();
				GlslConvert::OptimizationStruct::LowerVariableIndexToCondAssignOptions def;
				change |= ImGui::CheckBoxDefault("lower_input", &vProjectFile->m_OptimizationStruct.lowerVariableIndexToCondAssignOptions.lower_input, def.lower_input, 0);
				change |= ImGui::CheckBoxDefault("lower_output", &vProjectFile->m_OptimizationStruct.lowerVariableIndexToCondAssignOptions.lower_output, def.lower_output, 0);
				change |= ImGui::CheckBoxDefault("lower_temp", &vProjectFile->m_OptimizationStruct.lowerVariableIndexToCondAssignOptions.lower_temp, def.lower_temp, 0);
				change |= ImGui::CheckBoxDefault("lower_uniform", &vProjectFile->m_OptimizationStruct.lowerVariableIndexToCondAssignOptions.lower_uniform, def.lower_uniform, 0);
				ImGui::Unindent();
			}
			ImGui::Separator();
			CHECK("lower_instructions", 0, OPT_lower_instructions, true);
			if (vProjectFile->m_OptimizationStruct.optimizationFlags & GlslConvert::OptimizationFlags::OPT_lower_instructions)
			{
				ImGui::Indent();
				change |= DrawInstructionToLowerFlags(vProjectFile, vSize);
				ImGui::Unindent();
			}
			ImGui::Separator();
			CHECK("lower_jumps", 0, OPT_lower_jumps, true);
			if (vProjectFile->m_OptimizationStruct.optimizationFlags & GlslConvert::OptimizationFlags::OPT_lower_jumps)
			{
				ImGui::Indent();
				GlslConvert::OptimizationStruct::LowerJumpsOptions def;
				change |= ImGui::CheckBoxDefault("pull_out_jumps", &vProjectFile->m_OptimizationStruct.lowerJumpsOptions.pull_out_jumps, def.pull_out_jumps, 0);
				change |= ImGui::CheckBoxDefault("lower_sub_return", &vProjectFile->m_OptimizationStruct.lowerJumpsOptions.lower_sub_return, def.lower_sub_return, 0);
				change |= ImGui::CheckBoxDefault("lower_main_return", &vProjectFile->m_OptimizationStruct.lowerJumpsOptions.lower_main_return, def.lower_main_return, 0);
				change |= ImGui::CheckBoxDefault("lower_continue", &vProjectFile->m_OptimizationStruct.lowerJumpsOptions.lower_continue, def.lower_continue, 0);
				change |= ImGui::CheckBoxDefault("lower_break", &vProjectFile->m_OptimizationStruct.lowerJumpsOptions.lower_break, def.lower_break, 0);
				ImGui::Unindent();
			}
			ImGui::Separator();
			CHECK("lower_noise", 0, OPT_lower_noise, true);
			ImGui::Separator();
			CHECK("lower_quadop_vector", 0, OPT_lower_quadop_vector, true);
			ImGui::Separator();
			CHECK("lower_texture_projection", 0, OPT_lower_texture_projection, true);
			ImGui::Separator();
			CHECK("lower_if_to_cond_assign", "Try to Flatten if statement into Conditionnal Assignements\n ( like : x = (s > 0 ? a : b) )", OPT_lower_if_to_cond_assign, true);
			if (vProjectFile->m_OptimizationStruct.optimizationFlags & GlslConvert::OptimizationFlags::OPT_lower_if_to_cond_assign)
			{
				ImGui::Indent();
				GlslConvert::OptimizationStruct::LowerIfToCondAssignOptions def;
				change |= ImGui::SliderIntDefault(100, "max_depth", &vProjectFile->m_OptimizationStruct.lowerIfToCondAssignOptions.max_depth, 0, 120, 10);
				change |= ImGui::SliderIntDefault(100, "min_branch_cost", &vProjectFile->m_OptimizationStruct.lowerIfToCondAssignOptions.min_branch_cost, 0, 120, 1);
				ImGui::Unindent();
			}
			ImGui::Separator();
			CHECK("mat_op_to_vec", 0, OPT_mat_op_to_vec, true);
			ImGui::Separator();
			CHECK("optimize_swizzles", 0, OPT_optimize_swizzles, true);
			ImGui::Separator();
			CHECK("optimize_redundant_jumps", 0, OPT_optimize_redundant_jumps, true);
			ImGui::Separator();
			CHECK("structure_splitting", 0, OPT_structure_splitting, true);
			ImGui::Separator();
			CHECK("tree_grafting", 0, OPT_tree_grafting, true);
			ImGui::Separator();
			CHECK("vec_index_to_cond_assign", 0, OPT_vec_index_to_cond_assign, true);
			ImGui::Separator();
			CHECK("vec_index_to_swizzle", 0, OPT_vec_index_to_swizzle, true);
			ImGui::Separator();
			CHECK("flatten_nested_if_blocks", 0, OPT_flatten_nested_if_blocks, true);
			ImGui::Separator();
			CHECK("conditional_discard", 0, OPT_conditional_discard, true);
			ImGui::Separator();
			CHECK("flip_matrices", 0, OPT_flip_matrices, true);
			ImGui::Separator();
			CHECK("vectorize", 0, OPT_vectorize, true);
			ImGui::Separator();
			CHECK_BIS("minmax_prune", 0, OPT_minmax_prune, true);
			ImGui::Separator();
			CHECK_BIS("rebalance_tree", 0, OPT_rebalance_tree, true);
			ImGui::Separator();
			CHECK_BIS("lower_vector_insert", 0, OPT_lower_vector_insert, true);
			ImGui::Separator();
			CHECK_BIS("optimize_split_arrays", 0, OPT_optimize_split_arrays, true);
			ImGui::Separator();
			CHECK_BIS("set_unroll_Loops", 0, OPT_set_unroll_Loops, true);
		}
		ImGui::Unindent();
		ImGui::EndChild();
	}
#undef CHECK

	return change;
}

bool OptimizerPane::DrawCompilerFlags(ProjectFile *vProjectFile, ImVec2 vSize)
{
	bool change = false;

#define CHECK(STR, COM, FLAG, DEF) change |= ImGui::CheckBoxBitWize<GlslConvert::CompilerFlags>(STR, COM, &vProjectFile->m_OptimizationStruct.compilerFlags, GlslConvert::CompilerFlags::FLAG, DEF)
	if (ImGui::ImGui_CollapsingHeader_BitWize_OneAtATime<OptimizerPaneFlags>("Generation Settings", 
		-1, &m_OptimizerPaneFlags, OptimizerPaneFlags::OPT_PANE_COMPILER, false))
	{
		change |= DrawBitWizeToolBar<GlslConvert::CompilerFlags>(&vProjectFile->m_OptimizationStruct.compilerFlags);
		ImGui::BeginChild("##GenerationSettings");
		ImGui::Indent();
		{
			CHECK("EmitNoLoops", 0, COMPILER_EmitNoLoops, false);
			CHECK("EmitNoCont", 0, COMPILER_EmitNoCont, false);
			CHECK("EmitNoMainReturn", 0, COMPILER_EmitNoMainReturn, false);
			CHECK("EmitNoPow", 0, COMPILER_EmitNoPow, false);
			CHECK("EmitNoSat", 0, COMPILER_EmitNoSat, false);
			CHECK("LowerCombinedClipCullDistance", 0, COMPILER_LowerCombinedClipCullDistance, false);
			CHECK("EmitNoIndirectInput", 0, COMPILER_EmitNoIndirectInput, false);
			CHECK("EmitNoIndirectOutput", 0, COMPILER_EmitNoIndirectOutput, false);
			CHECK("EmitNoIndirectTemp", 0, COMPILER_EmitNoIndirectTemp, false);
			CHECK("EmitNoIndirectUniform", 0, COMPILER_EmitNoIndirectUniform, false);
			CHECK("EmitNoIndirectSampler", 0, COMPILER_EmitNoIndirectSampler, false);
			CHECK("OptimizeForAOS", 0, COMPILER_OptimizeForAOS, true);
			CHECK("LowerBufferInterfaceBlocks", 0, COMPILER_LowerBufferInterfaceBlocks, false);
			CHECK("ClampBlockIndicesToArrayBounds", 0, COMPILER_ClampBlockIndicesToArrayBounds, false);
			CHECK("PositionAlwaysInvariant", 0, COMPILER_PositionAlwaysInvariant, false);
		}
		ImGui::Unindent();
		ImGui::EndChild();
	}
#undef CHECK

	return change;
}

bool OptimizerPane::DrawInstructionToLowerFlags(ProjectFile *vProjectFile, ImVec2 vSize)
{
	bool change = false;

#define CHECK(STR, COM, FLAG, DEF) change |= ImGui::CheckBoxBitWize<GlslConvert::InstructionToLowerFlags>(STR, COM, &vProjectFile->m_OptimizationStruct.instructionToLowerFlags, GlslConvert::InstructionToLowerFlags::FLAG, DEF)
	change |= DrawBitWizeToolBar<GlslConvert::InstructionToLowerFlags>(&vProjectFile->m_OptimizationStruct.instructionToLowerFlags);
	CHECK("SUB_TO_ADD_NEG", 0, LOWER_SUB_TO_ADD_NEG, false);
	CHECK("EXP_TO_EXP2", 0, LOWER_EXP_TO_EXP2, false);
	CHECK("POW_TO_EXP2", 0, LOWER_POW_TO_EXP2, false);
	CHECK("LOG_TO_LOG2", 0, LOWER_LOG_TO_LOG2, false);
	CHECK("MOD_TO_FLOOR", 0, LOWER_MOD_TO_FLOOR, false);
	CHECK("INT_DIV_TO_MUL_RCP", 0, LOWER_INT_DIV_TO_MUL_RCP, false);
	CHECK("LDEXP_TO_ARITH", 0, LOWER_LDEXP_TO_ARITH, false);
	CHECK("CARRY_TO_ARITH", 0, LOWER_CARRY_TO_ARITH, false);
	CHECK("BORROW_TO_ARITH", 0, LOWER_BORROW_TO_ARITH, false);
	CHECK("SAT_TO_CLAMP", 0, LOWER_SAT_TO_CLAMP, false);
	CHECK("DOPS_TO_DFRAC", 0, LOWER_DOPS_TO_DFRAC, false);
	CHECK("DFREXP_DLDEXP_TO_ARITH", 0, LOWER_DFREXP_DLDEXP_TO_ARITH, false);
	CHECK("BIT_COUNT_TO_MATH", 0, LOWER_BIT_COUNT_TO_MATH, false);
	CHECK("EXTRACT_TO_SHIFTS", 0, LOWER_EXTRACT_TO_SHIFTS, false);
	CHECK("INSERT_TO_SHIFTS", 0, LOWER_INSERT_TO_SHIFTS, false);
	CHECK("REVERSE_TO_SHIFTS", 0, LOWER_REVERSE_TO_SHIFTS, false);
	CHECK("FIND_LSB_TO_FLOAT_CAST", 0, LOWER_FIND_LSB_TO_FLOAT_CAST, false);
	CHECK("FIND_MSB_TO_FLOAT_CAST", 0, LOWER_FIND_MSB_TO_FLOAT_CAST, false);
	CHECK("IMUL_HIGH_TO_MUL", 0, LOWER_IMUL_HIGH_TO_MUL, false);
	CHECK("DIV_TO_MUL_RCP", 0, LOWER_DIV_TO_MUL_RCP, false);
	if (vProjectFile->m_OptimizationStruct.instructionToLowerFlags & GlslConvert::InstructionToLowerFlags::LOWER_DIV_TO_MUL_RCP)
	{
		ImGui::Indent();
		CHECK("FDIV_TO_MUL_RCP", 0, LOWER_FDIV_TO_MUL_RCP, false);
		CHECK("DDIV_TO_MUL_RCP", 0, LOWER_DDIV_TO_MUL_RCP, false);
		ImGui::Unindent();
	}
	CHECK("SQRT_TO_ABS_SQRT", 0, LOWER_SQRT_TO_ABS_SQRT, false);
	CHECK("MUL64_TO_MUL_AND_MUL_HIGH", 0, LOWER_MUL64_TO_MUL_AND_MUL_HIGH, false);
#undef CHECK

	return change;
}

void OptimizerPane::Generate(ProjectFile *vProjectFile)
{
	std::string codeToOptimize = SourcePane::Instance()->GetCode();

	if (codeToOptimize.find("#version ") == std::string::npos)
	{
		codeToOptimize = m_Current_OpenGlVersionStruct.DefineCode + "\n\n" + codeToOptimize;
	}

	std::string optCode = GlslConvert::Instance()->Optimize(
		codeToOptimize,
		vProjectFile->m_ShaderStage,
		vProjectFile->m_ApiTarget,
		vProjectFile->m_LanguageTarget,
		m_Current_OpenGlVersionStruct.DefaultGlslVersionInt,
		vProjectFile->m_OptimizationStruct);

	TargetPane::Instance()->SetCode(optCode);
}

void OptimizerPane::ChangeGLSLVersionInCode(const std::string& vNewVersionCode)
{
	std::string version = vNewVersionCode;
	if (vNewVersionCode.empty())
		version = m_Current_OpenGlVersionStruct.DefineCode;

	std::string code = SourcePane::Instance()->GetCode();

	if (!code.empty())
	{
		size_t versionPos = code.find("#version ");
		if (versionPos != std::string::npos)
		{
			size_t endLine = code.find("\n", versionPos);
			if (endLine != std::string::npos)
			{
				std::string versionStr = code.substr(versionPos, endLine - versionPos);

				ct::replaceString(code, versionStr, vNewVersionCode);
			}
		}
	}
	else
	{
		code = version + "\n\n";
	}

	SourcePane::Instance()->SetCode(code);
}