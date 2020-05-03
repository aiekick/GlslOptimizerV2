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

#include "glsl_parser_extras.h"
#include "GlslConvert.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <share.h>
#include <getopt.h>

#include "ast.h"
#include "ir_optimization.h"
#include "program.h"
#include "program/program.h"
#include "ir_reader.h"
#include "standalone_scaffolding.h"
#include "main/mtypes.h"
#include "main/menums.h"
#include "builtin_functions.h"
#include "loop_analysis.h"

#include "ir_print_ir_visitor.h"
#include "ir_print_glsl_visitor.h"
#include "ir_builder_print_visitor.h"

#include "string_to_uint_map.h"
#include "linker.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

GlslConvert::GlslConvert()
{

}

GlslConvert::~GlslConvert()
{

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool GlslConvert::CreateGraph(
	std::string vShaderSource,
	ShaderStage vShaderType,
	ApiTarget vTarget,
	int vGLSLVersion,
	std::function<void(struct _mesa_glsl_parse_state*)> vFinishFunc)
{
	bool res = false;
	if (vShaderSource.empty()) return res;

	struct gl_shader *shader = rzalloc(NULL, struct gl_shader);

	shader->Stage = (gl_shader_stage)vShaderType;
	switch (shader->Stage)
	{
	case gl_shader_stage::MESA_SHADER_VERTEX:
		shader->Type = GL_VERTEX_SHADER;
		break;
	case gl_shader_stage::MESA_SHADER_TESS_CTRL:
		shader->Type = GL_TESS_CONTROL_SHADER;
		break;
	case gl_shader_stage::MESA_SHADER_TESS_EVAL:
		shader->Type = GL_TESS_EVALUATION_SHADER;
		break;
	case gl_shader_stage::MESA_SHADER_GEOMETRY:
		shader->Type = GL_GEOMETRY_SHADER;
		break;
	case gl_shader_stage::MESA_SHADER_FRAGMENT:
		shader->Type = GL_FRAGMENT_SHADER;
		break;
	case gl_shader_stage::MESA_SHADER_COMPUTE:
		shader->Type = GL_COMPUTE_SHADER;
		break;
	case gl_shader_stage::MESA_SHADER_KERNEL:
		// todo : opencl kernel target to generate after the others
		//shader->Type = GL_KERNEL_SHADER;
		break;
	default:
		break;
	}

	struct gl_context local_ctx;
	struct gl_context *ctx = &local_ctx;
	InitContext(ctx, vTarget, vGLSLVersion);

	ir_variable::temporaries_allocate_names = true;

	std::string input = vShaderSource;

	struct _mesa_glsl_parse_state *state
		= new(shader) _mesa_glsl_parse_state(ctx, shader->Stage, shader);

	shader->Source = input.c_str();
	const char *source = shader->Source;

	state->error = glcpp_preprocess(state, &source, &state->info_log, add_builtin_defines, state, ctx) != 0;

	if (!state->error)
	{
		_mesa_glsl_lexer_ctor(state, source);
		_mesa_glsl_parse(state);
		_mesa_glsl_lexer_dtor(state);
	}

	if (!state->error)
	{
		if (vFinishFunc)
		{
			vFinishFunc(state);
		}
	}
	else if (state->error)
	{
		res = state->info_log;
	}

	ralloc_free(state);
	ralloc_free(shader);

	ClearContext(ctx);

	return res;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

std::string GlslConvert::Optimize(
	std::string vShaderSource,
	ShaderStage vShaderType,
	ApiTarget vTarget,
	LanguageTarget vLanguageTarget,
	int vGLSLVersion,
	OptimizationStruct vOptimizationStruct)
{
	std::string res;
	if (vShaderSource.empty()) return res;
	
	struct gl_shader *shader = rzalloc(NULL, struct gl_shader);

	shader->Stage = (gl_shader_stage)vShaderType;
	switch (shader->Stage)
	{
	case gl_shader_stage::MESA_SHADER_VERTEX:
		shader->Type = GL_VERTEX_SHADER;
		break;
	case gl_shader_stage::MESA_SHADER_TESS_CTRL:
		shader->Type = GL_TESS_CONTROL_SHADER;
		break;
	case gl_shader_stage::MESA_SHADER_TESS_EVAL:
		shader->Type = GL_TESS_EVALUATION_SHADER;
		break;
	case gl_shader_stage::MESA_SHADER_GEOMETRY:
		shader->Type = GL_GEOMETRY_SHADER;
		break;
	case gl_shader_stage::MESA_SHADER_FRAGMENT:
		shader->Type = GL_FRAGMENT_SHADER;
		break;
	case gl_shader_stage::MESA_SHADER_COMPUTE:
		shader->Type = GL_COMPUTE_SHADER;
		break;
	case gl_shader_stage::MESA_SHADER_KERNEL:
		// todo : opencl kernel target to generate after the others
		//shader->Type = GL_KERNEL_SHADER;
		break;
	default:
		break;
	}
	
	vOptimizationStruct.stage = vShaderType;

	struct gl_context local_ctx;
	struct gl_context *ctx = &local_ctx;
	InitContext(ctx, vTarget, vGLSLVersion);

	gl_shader_compiler_options compileOptions =
		ctx->Const.ShaderCompilerOptions[(int)shader->Stage];
	FillCompilerOptions(&compileOptions, &vOptimizationStruct);

	ir_variable::temporaries_allocate_names = true;
		
	std::string input = vShaderSource;

	struct _mesa_glsl_parse_state *state
		= new(shader) _mesa_glsl_parse_state(ctx, shader->Stage, shader);

	struct gl_shader_program *program = 0;

	// si le format d'entréé est un ir
	//shader->ir = new(shader) exec_list;
	//_mesa_glsl_initialize_types(state);
	//_mesa_glsl_read_ir(state, shader->ir, input.c_str(), true);

	shader->Source = input.c_str();
	const char *source = shader->Source;

	if (!(vOptimizationStruct.controlFlags & ControlFlags::CONTROL_SKIP_PREPROCESSING))
	{
		state->error = glcpp_preprocess(state, &source, &state->info_log, add_builtin_defines, state, ctx) != 0;
	}

	if (!state->error)
	{
		_mesa_glsl_lexer_ctor(state, source);
		_mesa_glsl_parse(state);
		_mesa_glsl_lexer_dtor(state);
	
		if (vLanguageTarget == LanguageTarget::LANGUAGE_TARGET_AST)
		{
			//https://stackoverflow.com/questions/7664788/freopen-stdout-and-console
			/* Print out the initial AST */
			FILE *fp = freopen("tmp_ast", "w", stdout);
			if (fp)
			{
				foreach_list_typed(ast_node, ast, link, &state->translation_unit)
				{
					ast->print();
				}
				fclose(fp);
			}

			freopen("CON", "w", stdout);
			printf("Ast Export => SUCCESS\n");

			fp = fopen("tmp_ast", "r");
			if (fp)
			{
#define MAX_LENGTH 1024
				char *buffer = new char[MAX_LENGTH];
				while (!feof(fp)) 
				{
					fgets(buffer, MAX_LENGTH, fp);
					if (ferror(fp))
					{
						break;
					}
					else
					{
						res += buffer;
					}
				}

				delete[] buffer;
				fclose(fp);
			}
		}
		else
		{
			exec_list* ir = new (shader) exec_list();
			shader->ir = ir;

			if (!state->translation_unit.is_empty())
				_mesa_ast_to_hir(ir, state);

			if (!state->error)
			{
				// Link built-in functions
				shader->symbols = state->symbols;

				program = GetProgramFromShader(ctx, shader);

				if (program)
				{
					bool linked = false;

					if (!ir->is_empty() && !(vOptimizationStruct.controlFlags & ControlFlags::CONTROL_DO_PARTIAL_SHADER))
					{
						const gl_shader_stage stage = program->Shaders[0]->Stage;

						bool _allowMissingMain = true;
						program->data->LinkStatus = LINKING_SUCCESS;
						program->_LinkedShaders[stage] =
							link_intrastage_shaders(
								program /* mem_ctx */,
								ctx,
								program,
								program->Shaders,
								program->NumShaders,
								_allowMissingMain);

						if (program->_LinkedShaders[stage])
						{
							linked = true;

							struct gl_shader_compiler_options *const compiler_options =
								&ctx->Const.ShaderCompilerOptions[stage];

							ir = program->_LinkedShaders[stage]->ir;
						}
						else
						{
							linked = false;

							res = program->data->InfoLog;
						}
					}

					// Do optimization post-link
					DO_Optimization_Pass(
						ir,
						linked,
						&compileOptions,
						&vOptimizationStruct);

					validate_ir_tree(ir);

					/*if (vLanguageTarget == LanguageTarget::LANGUAGE_TARGET_IR_BUILDER)
					{
						const gl_shader_stage stage = program->Shaders[0]->Stage;
						FILE *fp = freopen("tmp_builder", "w", stdout);
						if (fp)
						{
							_mesa_print_builder_for_ir(stdout, ir);
							fclose(fp);
						}
						freopen("CON", "w", stdout);
						fp = fopen("tmp_builder", "r");
						if (fp)
						{
#define MAX_LENGTH 1024
							char *buffer = new char[MAX_LENGTH];
							while (!feof(fp))
							{
								fgets(buffer, MAX_LENGTH, fp);
								if (ferror(fp)) break;
								else res += buffer;
							}
							delete[] buffer;
							fclose(fp);
						}
					}*/
				}

				if (vLanguageTarget == LanguageTarget::LANGUAGE_TARGET_IR)
				{
					/* Print out the initial IR */
					res = IR_TO_IR::Convert(ir, state, ralloc_strdup(shader, ""));
				}
				else if (vLanguageTarget == LanguageTarget::LANGUAGE_TARGET_GLSL)
				{
					/* Print out the initial GLSL */
					res = IR_TO_GLSL::Convert(ir, state, ralloc_strdup(shader, ""));
				}
				/*else if (vLanguageTarget == LanguageTarget::LANGUAGE_TARGET_HLSL)
				{

				}
				else if (vLanguageTarget == LanguageTarget::LANGUAGE_TARGET_METAL)
				{

				}*/
			}
			else
			{
				res = state->info_log;
			}
		}
		
	}
	else
	{
		res = state->info_log;
	}
	
	// free
	if (program)
	{
		_mesa_clear_shader_program_data(ctx, program);
		ralloc_free(program);
	}
	ralloc_free(state);
	ralloc_free(shader);

	ClearContext(ctx);

	return res;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void GlslConvert::DO_Optimization_Pass(
	struct exec_list *vIr,
	bool linked,
	gl_shader_compiler_options *vCompilerFlags,
	OptimizationStruct *vOptimizationStruct)
{
#define OPT(FLAG, PASS, ...) do {																	\
	if ((vOptimizationStruct->optimizationFlags & OptimizationFlags::FLAG))	\
	progress |= PASS(__VA_ARGS__);																	\
	} while(false)																					\

#define OPT_BIS(FLAG, PASS, ...) do {																	\
	if ((vOptimizationStruct->optimizationFlags_Bis & OptimizationFlags_Bis::FLAG))	\
	progress |= PASS(__VA_ARGS__);																	\
	} while(false)																					\

	bool progress = false;
	int passes = 0;
	do {
		progress = false;
		++passes;
		
		if (vCompilerFlags && vOptimizationStruct)
		{
			OPT(OPT_lower_instructions, lower_instructions, 
				vIr, vOptimizationStruct->instructionToLowerFlags);
			if (linked)
			{
				OPT(OPT_function_inlining, do_function_inlining, vIr);
				OPT(OPT_dead_functions, do_dead_functions, vIr, 
					vOptimizationStruct->deadFunctionOptions.entryFunc.c_str());
				OPT(OPT_structure_splitting, do_structure_splitting, vIr);
			}
			propagate_invariance(vIr);
			OPT(OPT_if_simplification, do_if_simplification, vIr);
			OPT(OPT_flatten_nested_if_blocks, opt_flatten_nested_if_blocks, vIr);
			OPT(OPT_conditional_discard, opt_conditional_discard, vIr);
			OPT(OPT_copy_propagation_elements, do_copy_propagation_elements, vIr);
			if (vCompilerFlags->OptimizeForAOS && !linked)
				OPT(OPT_flip_matrices, opt_flip_matrices, vIr);
			if (linked && vCompilerFlags->OptimizeForAOS)
			{
				OPT(OPT_vectorize, do_vectorize, vIr);
			}
			if (linked)
				OPT(OPT_dead_code, do_dead_code, vIr,
					!vOptimizationStruct->deadCodeOptions.keep_only_assigned_uniforms);
			else
				OPT(OPT_dead_code_unlinked, do_dead_code_unlinked, vIr);
			OPT(OPT_dead_code_local, do_dead_code_local, vIr);
			OPT(OPT_tree_grafting, do_tree_grafting, vIr);
			OPT(OPT_constant_propagation, do_constant_propagation, vIr);
			if (linked)
				OPT(OPT_constant_variable, do_constant_variable, vIr);
			else
				OPT(OPT_constant_variable_unlinked, do_constant_variable_unlinked, vIr);
			OPT(OPT_constant_folding, do_constant_folding, vIr);
			OPT_BIS(OPT_minmax_prune, do_minmax_prune, vIr);
			OPT_BIS(OPT_rebalance_tree, do_rebalance_tree, vIr);
			OPT(OPT_algebraic, do_algebraic, vIr,
				vOptimizationStruct->algebraicOptions.native_integers, vCompilerFlags);
			OPT(OPT_lower_jumps, do_lower_jumps, vIr,
				vOptimizationStruct->lowerJumpsOptions.pull_out_jumps,
				vOptimizationStruct->lowerJumpsOptions.lower_sub_return,
				vOptimizationStruct->lowerJumpsOptions.lower_main_return,
				vOptimizationStruct->lowerJumpsOptions.lower_continue,
				vOptimizationStruct->lowerJumpsOptions.lower_break);
			OPT(OPT_vec_index_to_swizzle, do_vec_index_to_swizzle, vIr);
			OPT_BIS(OPT_lower_vector_insert, lower_vector_insert, vIr,
				vOptimizationStruct->lowerVectorInsertOptions.lower_nonconstant_index);
			OPT(OPT_optimize_swizzles, optimize_swizzles, vIr);
			OPT_BIS(OPT_optimize_split_arrays, optimize_split_arrays, vIr, linked);
			OPT(OPT_optimize_redundant_jumps, optimize_redundant_jumps, vIr);
			if (OPT_BIS_FLAGS(vOptimizationStruct->optimizationFlags_Bis, OPT_set_unroll_Loops))
			{
				if (vCompilerFlags->MaxUnrollIterations)
				{
					loop_state *ls = analyze_loop_variables(vIr);
					if (ls->loop_found)
					{
						bool loop_progress = unroll_loops(vIr, ls, vCompilerFlags);
						while (loop_progress)
						{
							loop_progress = false;
							loop_progress |= do_constant_propagation(vIr);
							loop_progress |= do_if_simplification(vIr);

							/* Some drivers only call do_common_optimization() once rather
							 * than in a loop. So we must call do_lower_jumps() after
							 * unrolling a loop because for drivers that use LLVM validation
							 * will fail if a jump is not the last instruction in the block.
							 * For example the following will fail LLVM validation:
							 *
							 *   (loop (
							 *      ...
							 *   break
							 *   (assign  (x) (var_ref v124)  (expression int + (var_ref v124)
							 *      (constant int (1)) ) )
							 *   ))
							 */
							loop_progress |= do_lower_jumps(vIr, 
								true, 
								true,
								vCompilerFlags->EmitNoMainReturn,
								vCompilerFlags->EmitNoCont,
								vCompilerFlags->EmitNoLoops);
						}
						progress |= loop_progress;
					}
					delete ls;
				}
			}
			OPT(OPT_lower_texture_projection, do_lower_texture_projection, vIr);
			if (OPT_FLAGS(vOptimizationStruct->optimizationFlags, OPT_lower_if_to_cond_assign))
			{
				gl_shader_stage stage = (gl_shader_stage)vOptimizationStruct->stage;
				progress |= lower_if_to_cond_assign(stage, vIr, 
					vOptimizationStruct->lowerIfToCondAssignOptions.max_depth, 
					vOptimizationStruct->lowerIfToCondAssignOptions.min_branch_cost);
			}
			OPT(OPT_mat_op_to_vec, do_mat_op_to_vec, vIr);
			OPT(OPT_vec_index_to_cond_assign, do_vec_index_to_cond_assign, vIr);
			OPT(OPT_lower_discard, lower_discard, vIr);
			OPT(OPT_lower_noise, lower_noise, vIr);
			if (OPT_FLAGS(vOptimizationStruct->optimizationFlags, OPT_lower_variable_index_to_cond_assign))
			{
				gl_shader_stage stage = (gl_shader_stage)vOptimizationStruct->stage;
				progress |= lower_variable_index_to_cond_assign(
					stage, vIr,
					vOptimizationStruct->lowerVariableIndexToCondAssignOptions.lower_input, 
					vOptimizationStruct->lowerVariableIndexToCondAssignOptions.lower_output,
					vOptimizationStruct->lowerVariableIndexToCondAssignOptions.lower_temp, 
					vOptimizationStruct->lowerVariableIndexToCondAssignOptions.lower_uniform);
			}
			OPT(OPT_lower_quadop_vector, lower_quadop_vector, vIr, vOptimizationStruct->lowerQuadopVector.dont_lower_swz);

			validate_ir_tree(vIr);
		}
	} while (progress && passes < vOptimizationStruct->maxCountPasses);
#undef OPT
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void init_gl_program(struct gl_program *prog, bool is_arb_asm, GLenum target)
{
	prog->RefCount = 1;
	prog->Format = GL_PROGRAM_FORMAT_ASCII_ARB;
	prog->is_arb_asm = is_arb_asm;
	prog->info.stage = (gl_shader_stage)_mesa_program_enum_to_shader_stage(target);
}

static struct gl_program* new_program(UNUSED struct gl_context *ctx, GLenum target,
	UNUSED GLuint id, bool is_arb_asm)
{
	switch (target) {
	case GL_VERTEX_PROGRAM_ARB: /* == GL_VERTEX_PROGRAM_NV */
	case GL_GEOMETRY_PROGRAM_NV:
	case GL_TESS_CONTROL_PROGRAM_NV:
	case GL_TESS_EVALUATION_PROGRAM_NV:
	case GL_FRAGMENT_PROGRAM_ARB:
	case GL_COMPUTE_PROGRAM_NV: {
		struct gl_program *prog = rzalloc(NULL, struct gl_program);
		init_gl_program(prog, is_arb_asm, target);
		return prog;
	}
	default:
		printf("bad target in new_program\n");
		return NULL;
	}
}

void GlslConvert::InitContext(struct gl_context *ctx, ApiTarget api, int vGlslVersion)
{
	gl_api glApi;
	if (vGlslVersion == 100 || vGlslVersion == 300)
		glApi = gl_api::API_OPENGLES2;
	else if (api == ApiTarget::API_OPENGL_COMPAT)
		glApi = gl_api::API_OPENGL_COMPAT;
	else if (api == ApiTarget::API_OPENGL_CORE)
		glApi = gl_api::API_OPENGL_CORE;

	initialize_context_to_defaults(ctx, glApi);

	_mesa_glsl_builtin_functions_init_or_ref();

	/* The standalone compiler needs to claim support for almost
	 * everything in order to compile the built-in functions.
	 */
	ctx->Const.GLSLVersion = vGlslVersion;

	ctx->Extensions.ARB_ES3_compatibility = true;
	ctx->Extensions.ARB_ES3_1_compatibility = true;
	ctx->Extensions.ARB_ES3_2_compatibility = true;

	ctx->Const.MaxComputeWorkGroupCount[0] = 65535;
	ctx->Const.MaxComputeWorkGroupCount[1] = 65535;
	ctx->Const.MaxComputeWorkGroupCount[2] = 65535;
	ctx->Const.MaxComputeWorkGroupSize[0] = 1024;
	ctx->Const.MaxComputeWorkGroupSize[1] = 1024;
	ctx->Const.MaxComputeWorkGroupSize[2] = 64;
	ctx->Const.MaxComputeWorkGroupInvocations = 1024;
	ctx->Const.MaxComputeSharedMemorySize = 32768;
	ctx->Const.MaxComputeVariableGroupSize[0] = 512;
	ctx->Const.MaxComputeVariableGroupSize[1] = 512;
	ctx->Const.MaxComputeVariableGroupSize[2] = 64;
	ctx->Const.MaxComputeVariableGroupInvocations = 512;
	ctx->Const.Program[MESA_SHADER_COMPUTE].MaxTextureImageUnits = 16;
	ctx->Const.Program[MESA_SHADER_COMPUTE].MaxUniformComponents = 1024;
	ctx->Const.Program[MESA_SHADER_COMPUTE].MaxCombinedUniformComponents = 1024;
	ctx->Const.Program[MESA_SHADER_COMPUTE].MaxInputComponents = 0; /* not used */
	ctx->Const.Program[MESA_SHADER_COMPUTE].MaxOutputComponents = 0; /* not used */
	ctx->Const.Program[MESA_SHADER_COMPUTE].MaxAtomicBuffers = 8;
	ctx->Const.Program[MESA_SHADER_COMPUTE].MaxAtomicCounters = 8;
	ctx->Const.Program[MESA_SHADER_COMPUTE].MaxImageUniforms = 8;
	ctx->Const.Program[MESA_SHADER_COMPUTE].MaxUniformBlocks = 12;

	switch (ctx->Const.GLSLVersion) {
	case 100:
		ctx->Const.MaxClipPlanes = 0;
		ctx->Const.MaxCombinedTextureImageUnits = 8;
		ctx->Const.MaxDrawBuffers = 2;
		ctx->Const.MinProgramTexelOffset = 0;
		ctx->Const.MaxProgramTexelOffset = 0;
		ctx->Const.MaxLights = 0;
		ctx->Const.MaxTextureCoordUnits = 0;
		ctx->Const.MaxTextureUnits = 8;

		ctx->Const.Program[MESA_SHADER_VERTEX].MaxAttribs = 8;
		ctx->Const.Program[MESA_SHADER_VERTEX].MaxTextureImageUnits = 0;
		ctx->Const.Program[MESA_SHADER_VERTEX].MaxUniformComponents = 128 * 4;
		ctx->Const.Program[MESA_SHADER_VERTEX].MaxCombinedUniformComponents = 128 * 4;
		ctx->Const.Program[MESA_SHADER_VERTEX].MaxInputComponents = 0; /* not used */
		ctx->Const.Program[MESA_SHADER_VERTEX].MaxOutputComponents = 32;

		ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxTextureImageUnits =
			ctx->Const.MaxCombinedTextureImageUnits;
		ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxUniformComponents = 16 * 4;
		ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxCombinedUniformComponents = 16 * 4;
		ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxInputComponents =
			ctx->Const.Program[MESA_SHADER_VERTEX].MaxOutputComponents;
		ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxOutputComponents = 0; /* not used */

		ctx->Const.MaxVarying = ctx->Const.Program[MESA_SHADER_VERTEX].MaxOutputComponents / 4;
		break;
	case 110:
	case 120:
		ctx->Const.MaxClipPlanes = 6;
		ctx->Const.MaxCombinedTextureImageUnits = 2;
		ctx->Const.MaxDrawBuffers = 1;
		ctx->Const.MinProgramTexelOffset = 0;
		ctx->Const.MaxProgramTexelOffset = 0;
		ctx->Const.MaxLights = 8;
		ctx->Const.MaxTextureCoordUnits = 2;
		ctx->Const.MaxTextureUnits = 2;

		ctx->Const.Program[MESA_SHADER_VERTEX].MaxAttribs = 16;
		ctx->Const.Program[MESA_SHADER_VERTEX].MaxTextureImageUnits = 0;
		ctx->Const.Program[MESA_SHADER_VERTEX].MaxUniformComponents = 512;
		ctx->Const.Program[MESA_SHADER_VERTEX].MaxCombinedUniformComponents = 512;
		ctx->Const.Program[MESA_SHADER_VERTEX].MaxInputComponents = 0; /* not used */
		ctx->Const.Program[MESA_SHADER_VERTEX].MaxOutputComponents = 32;

		ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxTextureImageUnits =
			ctx->Const.MaxCombinedTextureImageUnits;
		ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxUniformComponents = 64;
		ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxCombinedUniformComponents = 64;
		ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxInputComponents =
			ctx->Const.Program[MESA_SHADER_VERTEX].MaxOutputComponents;
		ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxOutputComponents = 0; /* not used */

		ctx->Const.MaxVarying = ctx->Const.Program[MESA_SHADER_VERTEX].MaxOutputComponents / 4;
		break;
	case 130:
	case 140:
		ctx->Const.MaxClipPlanes = 8;
		ctx->Const.MaxCombinedTextureImageUnits = 16;
		ctx->Const.MaxDrawBuffers = 8;
		ctx->Const.MinProgramTexelOffset = -8;
		ctx->Const.MaxProgramTexelOffset = 7;
		ctx->Const.MaxLights = 8;
		ctx->Const.MaxTextureCoordUnits = 8;
		ctx->Const.MaxTextureUnits = 2;
		ctx->Const.MaxUniformBufferBindings = 84;
		ctx->Const.MaxVertexStreams = 4;
		ctx->Const.MaxTransformFeedbackBuffers = 4;

		ctx->Const.Program[MESA_SHADER_VERTEX].MaxAttribs = 16;
		ctx->Const.Program[MESA_SHADER_VERTEX].MaxTextureImageUnits = 16;
		ctx->Const.Program[MESA_SHADER_VERTEX].MaxUniformComponents = 1024;
		ctx->Const.Program[MESA_SHADER_VERTEX].MaxCombinedUniformComponents = 1024;
		ctx->Const.Program[MESA_SHADER_VERTEX].MaxInputComponents = 0; /* not used */
		ctx->Const.Program[MESA_SHADER_VERTEX].MaxOutputComponents = 64;

		ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxTextureImageUnits = 16;
		ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxUniformComponents = 1024;
		ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxCombinedUniformComponents = 1024;
		ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxInputComponents =
			ctx->Const.Program[MESA_SHADER_VERTEX].MaxOutputComponents;
		ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxOutputComponents = 0; /* not used */

		ctx->Const.MaxVarying = ctx->Const.Program[MESA_SHADER_VERTEX].MaxOutputComponents / 4;
		break;
	case 150:
	case 330:
	case 400:
	case 410:
	case 420:
	case 430:
	case 440:
	case 450:
	case 460:
		ctx->Const.MaxClipPlanes = 8;
		ctx->Const.MaxDrawBuffers = 8;
		ctx->Const.MinProgramTexelOffset = -8;
		ctx->Const.MaxProgramTexelOffset = 7;
		ctx->Const.MaxLights = 8;
		ctx->Const.MaxTextureCoordUnits = 8;
		ctx->Const.MaxTextureUnits = 2;
		ctx->Const.MaxUniformBufferBindings = 84;
		ctx->Const.MaxVertexStreams = 4;
		ctx->Const.MaxTransformFeedbackBuffers = 4;
		ctx->Const.MaxShaderStorageBufferBindings = 4;
		ctx->Const.MaxShaderStorageBlockSize = 4096;
		ctx->Const.MaxAtomicBufferBindings = 4;

		ctx->Const.Program[MESA_SHADER_VERTEX].MaxAttribs = 16;
		ctx->Const.Program[MESA_SHADER_VERTEX].MaxTextureImageUnits = 16;
		ctx->Const.Program[MESA_SHADER_VERTEX].MaxUniformComponents = 1024;
		ctx->Const.Program[MESA_SHADER_VERTEX].MaxCombinedUniformComponents = 1024;
		ctx->Const.Program[MESA_SHADER_VERTEX].MaxInputComponents = 0; /* not used */
		ctx->Const.Program[MESA_SHADER_VERTEX].MaxOutputComponents = 64;

		ctx->Const.Program[MESA_SHADER_GEOMETRY].MaxTextureImageUnits = 16;
		ctx->Const.Program[MESA_SHADER_GEOMETRY].MaxUniformComponents = 1024;
		ctx->Const.Program[MESA_SHADER_GEOMETRY].MaxCombinedUniformComponents = 1024;
		ctx->Const.Program[MESA_SHADER_GEOMETRY].MaxInputComponents =
			ctx->Const.Program[MESA_SHADER_VERTEX].MaxOutputComponents;
		ctx->Const.Program[MESA_SHADER_GEOMETRY].MaxOutputComponents = 128;

		ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxTextureImageUnits = 16;
		ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxUniformComponents = 1024;
		ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxCombinedUniformComponents = 1024;
		ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxInputComponents =
			ctx->Const.Program[MESA_SHADER_GEOMETRY].MaxOutputComponents;
		ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxOutputComponents = 0; /* not used */

		ctx->Const.MaxCombinedTextureImageUnits =
			ctx->Const.Program[MESA_SHADER_VERTEX].MaxTextureImageUnits
			+ ctx->Const.Program[MESA_SHADER_GEOMETRY].MaxTextureImageUnits
			+ ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxTextureImageUnits;

		ctx->Const.MaxGeometryOutputVertices = 256;
		ctx->Const.MaxGeometryTotalOutputComponents = 1024;

		ctx->Const.MaxVarying = 60 / 4;
		break;
	case 300:
		ctx->Const.MaxClipPlanes = 8;
		ctx->Const.MaxCombinedTextureImageUnits = 32;
		ctx->Const.MaxDrawBuffers = 4;
		ctx->Const.MinProgramTexelOffset = -8;
		ctx->Const.MaxProgramTexelOffset = 7;
		ctx->Const.MaxLights = 0;
		ctx->Const.MaxTextureCoordUnits = 0;
		ctx->Const.MaxTextureUnits = 0;
		ctx->Const.MaxUniformBufferBindings = 84;
		ctx->Const.MaxVertexStreams = 4;
		ctx->Const.MaxTransformFeedbackBuffers = 4;

		ctx->Const.Program[MESA_SHADER_VERTEX].MaxAttribs = 16;
		ctx->Const.Program[MESA_SHADER_VERTEX].MaxTextureImageUnits = 16;
		ctx->Const.Program[MESA_SHADER_VERTEX].MaxUniformComponents = 1024;
		ctx->Const.Program[MESA_SHADER_VERTEX].MaxCombinedUniformComponents = 1024;
		ctx->Const.Program[MESA_SHADER_VERTEX].MaxInputComponents = 0; /* not used */
		ctx->Const.Program[MESA_SHADER_VERTEX].MaxOutputComponents = 16 * 4;

		ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxTextureImageUnits = 16;
		ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxUniformComponents = 224;
		ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxCombinedUniformComponents = 224;
		ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxInputComponents = 15 * 4;
		ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxOutputComponents = 0; /* not used */

		ctx->Const.MaxVarying = ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxInputComponents / 4;
		break;
	}

	ctx->Const.GenerateTemporaryNames = true;
	ctx->Const.MaxPatchVertices = 32;

	/* GL_ARB_explicit_uniform_location, GL_MAX_UNIFORM_LOCATIONS */
	ctx->Const.MaxUserAssignableUniformLocations =
		4 * MESA_SHADER_STAGES * MAX_UNIFORMS;

	ctx->Driver.NewProgram = new_program;
	//ctx->Driver.DeleteProgram = 0;
}

void GlslConvert::ClearContext(struct gl_context *ctx)
{
	_mesa_glsl_builtin_functions_decref();
}

struct gl_shader_program* GlslConvert::GetProgramFromShader(struct gl_context *ctx, struct gl_shader *shader)
{
	struct gl_shader_program *whole_program = 0;

	if (!ctx) return whole_program;
	if (!shader) return whole_program;
	
	whole_program = rzalloc(NULL, struct gl_shader_program);
	assert(whole_program != NULL);
	whole_program->data = rzalloc(whole_program, struct gl_shader_program_data);
	assert(whole_program->data != NULL);
	whole_program->data->InfoLog = ralloc_strdup(whole_program->data, "");

	/* Created just to avoid segmentation faults */
	whole_program->AttributeBindings = new string_to_uint_map;
	whole_program->FragDataBindings = new string_to_uint_map;
	whole_program->FragDataIndexBindings = new string_to_uint_map;

	// attach
	whole_program->Shaders =
		reralloc(whole_program, whole_program->Shaders,
			struct gl_shader *, whole_program->NumShaders + 1);
	assert(whole_program->Shaders != NULL);

	whole_program->Shaders[whole_program->NumShaders] = shader;
	whole_program->NumShaders++;
	
	return whole_program;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void GlslConvert::FillCompilerOptions(gl_shader_compiler_options *vCompileOptions, OptimizationStruct *vOptimizationStruct)
{
	if (vCompileOptions && vOptimizationStruct)
	{
#define COND(FLAG) (&vOptimizationStruct->compilerFlags && GlslConvert::CompilerFlags::FLAG)
		vCompileOptions->EmitNoLoops = COND(COMPILER_EmitNoLoops);
		vCompileOptions->EmitNoCont = COND(COMPILER_EmitNoCont);
		vCompileOptions->EmitNoMainReturn = COND(COMPILER_EmitNoMainReturn);
		vCompileOptions->EmitNoPow = COND(COMPILER_EmitNoPow);
		vCompileOptions->EmitNoSat = COND(COMPILER_EmitNoSat);
		vCompileOptions->LowerCombinedClipCullDistance = COND(COMPILER_LowerCombinedClipCullDistance);
		vCompileOptions->EmitNoIndirectInput = COND(COMPILER_EmitNoIndirectInput);
		vCompileOptions->EmitNoIndirectOutput = COND(COMPILER_EmitNoIndirectOutput);
		vCompileOptions->EmitNoIndirectTemp = COND(COMPILER_EmitNoIndirectTemp);
		vCompileOptions->EmitNoIndirectUniform = COND(COMPILER_EmitNoIndirectUniform);
		vCompileOptions->EmitNoIndirectSampler = COND(COMPILER_EmitNoIndirectSampler);
		vCompileOptions->MaxIfDepth = vOptimizationStruct->instructionToLower.MaxIfDepth;
		vCompileOptions->MaxUnrollIterations = vOptimizationStruct->instructionToLower.MaxUnrollIterations;
		vCompileOptions->OptimizeForAOS = COND(COMPILER_OptimizeForAOS);
		vCompileOptions->LowerBufferInterfaceBlocks = COND(COMPILER_LowerBufferInterfaceBlocks);
		vCompileOptions->ClampBlockIndicesToArrayBounds = COND(COMPILER_ClampBlockIndicesToArrayBounds);
		vCompileOptions->PositionAlwaysInvariant = COND(COMPILER_PositionAlwaysInvariant);
#undef COND
	}
}