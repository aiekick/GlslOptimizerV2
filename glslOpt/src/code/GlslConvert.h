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

#pragma once;

#include "compiler/shader_enums.h"
#include <string>
#include <map>
#include <functional>

struct exec_list;
struct gl_context;
struct gl_shader_compiler_options;
struct _mesa_glsl_parse_state;
class GlslConvert
{
public:

	enum ShaderStage
	{
		MESA_SHADER_NONE = -1,
		MESA_SHADER_VERTEX = 0,
		MESA_SHADER_TESS_CTRL = 1,
		MESA_SHADER_TESS_EVAL = 2,
		MESA_SHADER_GEOMETRY = 3,
		MESA_SHADER_FRAGMENT = 4,
		MESA_SHADER_COMPUTE = 5,
		/* must be last so it doesn't affect the GL pipeline */
		MESA_SHADER_KERNEL = 6,
	};

	enum ApiTarget
	{
		API_OPENGL_COMPAT = 0,
		API_OPENGL_CORE
	};

	enum LanguageTarget
	{
		LANGUAGE_TARGET_AST = 0,
		LANGUAGE_TARGET_IR,
		//LANGUAGE_TARGET_IR_BUILDER,
		LANGUAGE_TARGET_GLSL,
		//LANGUAGE_TARGET_HLSL,
		//LANGUAGE_TARGET_METAL
	};

	enum ControlFlags 
	{
		CONTROL_SKIP_PREPROCESSING = (1 << 0), // Skip preprocessing shader source. Saves some time if you know you don't need it.
		CONTROL_DO_PARTIAL_SHADER = (1 << 1) // Passed shader is not the full shader source. This makes some optimizations weaker.
	};

	enum CompilerFlags
	{
		COMPILER_EmitNoLoops					= (1 << 0),
		COMPILER_EmitNoCont						= (1 << 1),
		COMPILER_EmitNoMainReturn				= (1 << 2),
		COMPILER_EmitNoPow						= (1 << 3),
		COMPILER_EmitNoSat						= (1 << 4),
		COMPILER_LowerCombinedClipCullDistance	= (1 << 5),
		COMPILER_EmitNoIndirectInput			= (1 << 6),
		COMPILER_EmitNoIndirectOutput			= (1 << 7),
		COMPILER_EmitNoIndirectTemp				= (1 << 8),
		COMPILER_EmitNoIndirectUniform			= (1 << 9),
		COMPILER_EmitNoIndirectSampler			= (1 << 10),
		COMPILER_OptimizeForAOS					= (1 << 11),
		COMPILER_LowerBufferInterfaceBlocks		= (1 << 12),
		COMPILER_ClampBlockIndicesToArrayBounds = (1 << 13),
		COMPILER_PositionAlwaysInvariant		= (1 << 14)
	};

	enum InstructionToLowerFlags
	{
		LOWER_SUB_TO_ADD_NEG			= (1 << 0),
		LOWER_FDIV_TO_MUL_RCP			= (1 << 1),
		LOWER_EXP_TO_EXP2				= (1 << 2),
		LOWER_POW_TO_EXP2				= (1 << 3),
		LOWER_LOG_TO_LOG2				= (1 << 4),
		LOWER_MOD_TO_FLOOR				= (1 << 5),
		LOWER_INT_DIV_TO_MUL_RCP		= (1 << 6),
		LOWER_LDEXP_TO_ARITH			= (1 << 7),
		LOWER_CARRY_TO_ARITH			= (1 << 8),
		LOWER_BORROW_TO_ARITH			= (1 << 9),
		LOWER_SAT_TO_CLAMP				= (1 << 10),
		LOWER_DOPS_TO_DFRAC				= (1 << 11),
		LOWER_DFREXP_DLDEXP_TO_ARITH	= (1 << 12),
		LOWER_BIT_COUNT_TO_MATH			= (1 << 13),
		LOWER_EXTRACT_TO_SHIFTS			= (1 << 14),
		LOWER_INSERT_TO_SHIFTS			= (1 << 15),
		LOWER_REVERSE_TO_SHIFTS			= (1 << 16),
		LOWER_FIND_LSB_TO_FLOAT_CAST	= (1 << 17),
		LOWER_FIND_MSB_TO_FLOAT_CAST	= (1 << 18),
		LOWER_IMUL_HIGH_TO_MUL			= (1 << 19),
		LOWER_DDIV_TO_MUL_RCP			= (1 << 20),
		LOWER_DIV_TO_MUL_RCP			= (LOWER_FDIV_TO_MUL_RCP | LOWER_DDIV_TO_MUL_RCP),
		LOWER_SQRT_TO_ABS_SQRT			= (1 << 21),
		LOWER_MUL64_TO_MUL_AND_MUL_HIGH = (1 << 22)

	};
#define OPT_FLAGS(CONTAINER, FLAG_TO_TEST) (CONTAINER & OptimizationFlags::FLAG_TO_TEST)
	enum OptimizationFlags : long long // 64 bits max donc 63 en partant de 0
	{
		OPT_algebraic								= (1LL << 0),	// c/o
		OPT_common_optimization						= (1LL << 1),
		OPT_constant_folding						= (1LL << 2),	// c/o
		OPT_constant_propagation					= (1LL << 3),	// c/o
		OPT_constant_variable						= (1LL << 4),	// c/o
		OPT_constant_variable_unlinked				= (1LL << 5),	// c/o
		OPT_copy_propagation_elements				= (1LL << 6),	// c/o
		OPT_dead_code								= (1LL << 7),	// c/o
		OPT_dead_code_local							= (1LL << 8),	// c/o
		OPT_dead_code_unlinked						= (1LL << 9),	// c/o
		OPT_dead_functions							= (1LL << 10),	// c/o
		OPT_function_inlining						= (1LL << 11),	// c/o
		OPT_if_simplification						= (1LL << 12),	// c/o
		OPT_lower_discard							= (1LL << 13),
		OPT_lower_variable_index_to_cond_assign		= (1LL << 14),
		OPT_lower_instructions						= (1LL << 15),
		OPT_lower_jumps								= (1LL << 16),	// c/o
		OPT_lower_noise								= (1LL << 17),
		OPT_lower_quadop_vector						= (1LL << 18),
		OPT_lower_texture_projection				= (1LL << 19),
		OPT_lower_if_to_cond_assign					= (1LL << 20),
		OPT_mat_op_to_vec							= (1LL << 21),
		OPT_optimize_swizzles						= (1LL << 22),
		OPT_optimize_redundant_jumps				= (1LL << 23),	// c/o
		OPT_structure_splitting						= (1LL << 24),	// c/o
		OPT_tree_grafting							= (1LL << 25),	// c/o
		OPT_vec_index_to_cond_assign				= (1LL << 26),
		OPT_vec_index_to_swizzle					= (1LL << 27),	// c/o
		OPT_flatten_nested_if_blocks				= (1LL << 28),
		OPT_conditional_discard						= (1LL << 29),
		OPT_flip_matrices							= (1LL << 30),
		OPT_vectorize								= (1LL << 31),
		OPT_minmax_prune							= (1LL << 32),
		OPT_rebalance_tree							= (1LL << 33),
		OPT_lower_vector_insert						= (1LL << 34),
		OPT_optimize_split_arrays					= (1LL << 35),
		OPT_set_unroll_Loops						= (1LL << 36),
	};
	
	struct OptimizationStruct
	{
		// dont save
		int maxCountPasses = 1000;
		ShaderStage stage = ShaderStage::MESA_SHADER_FRAGMENT;

		// the rest is to save
		CompilerFlags compilerFlags = (GlslConvert::CompilerFlags)0;
		ControlFlags controlFlags = (GlslConvert::ControlFlags)0;
		OptimizationFlags optimizationFlags = (GlslConvert::OptimizationFlags)~0; // all
		InstructionToLowerFlags instructionToLowerFlags = (GlslConvert::InstructionToLowerFlags)~0; // all
		
		struct AlgebraicOptions
		{
			bool native_integers = true;
		} algebraicOptions;

		struct LowerJumpsOptions
		{
			bool pull_out_jumps = true;
			bool lower_sub_return = true;
			bool lower_main_return = true;
			bool lower_continue = true;
			bool lower_break = true;
		} lowerJumpsOptions;

		struct LowerIfToCondAssignOptions
		{
			int max_depth = 10;
			int min_branch_cost = 1;
		} lowerIfToCondAssignOptions;

		struct LowerVariableIndexToCondAssignOptions
		{
			bool lower_input = true;
			bool lower_output = true;
			bool lower_temp = true;
			bool lower_uniform = true;
		} lowerVariableIndexToCondAssignOptions;

		struct DeadCodeOptions
		{
			bool keep_only_assigned_uniforms = true; // true => ne garde que les uniform assignés (loc >= 0) 
		} deadCodeOptions;

		struct DeadFunctionOptions
		{
			std::string entryFunc = "main";
		} deadFunctionOptions;

		struct LowerVectorInsertOptions
		{
			bool lower_nonconstant_index = false;
		} lowerVectorInsertOptions;

		struct LowerQuadopVector
		{
			bool dont_lower_swz = true;
		} lowerQuadopVector;

		struct InstructionToLower
		{
			int MaxIfDepth = 10;
			int MaxUnrollIterations = 10;
		} instructionToLower;
	};

public:
	static GlslConvert* Instance()
	{
		static GlslConvert *_instance = new GlslConvert();
		return _instance;
	}

protected:
	GlslConvert(); // Prevent construction
	GlslConvert(const GlslConvert&) {}; // Prevent construction by copying
	GlslConvert& operator =(const GlslConvert&) { return *this; }; // Prevent assignment
	~GlslConvert(); // Prevent unwanted destruction

public:
	bool CreateGraph(
		std::string vShaderSource,
		ShaderStage vShaderType,
		ApiTarget vTarget,
		int vGLSLVersion,
		std::function<void(struct _mesa_glsl_parse_state*)> vFinishFunc);
	
	std::string Optimize(
		std::string vShaderSource, 
		ShaderStage vShaderType,
		ApiTarget vTarget, 
		LanguageTarget vLanguageTarget, 
		int vGLSLVersion, 
		OptimizationStruct vOptimisationStruct);

private:
	void DO_Optimization_Pass(
		struct exec_list *vIr, 
		bool linked,
		gl_shader_compiler_options *vCompilerFlags,
		OptimizationStruct *vOptimisationStruct);

public:
	static void InitContext(struct gl_context *ctx, ApiTarget api, int vGlslVersion);
	static void ClearContext(struct gl_context *ctx);
	static struct gl_shader_program* GetProgramFromShader(struct gl_context *ctx, struct gl_shader *shader);

private:
	void FillCompilerOptions(gl_shader_compiler_options *vCompileOptions, OptimizationStruct *vOptimizationStruct);
};
