cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0012 NEW)
cmake_policy(SET CMP0023 NEW)

enable_language(C CXX)

file(GLOB PROJECT_SRC 
${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)
file(GLOB PROJECT_SRC_COMPILER
${CMAKE_CURRENT_SOURCE_DIR}/src/compiler/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/src/compiler/*.c
${CMAKE_CURRENT_SOURCE_DIR}/src/compiler/*.h)
file(GLOB PROJECT_SRC_GETOPT
${CMAKE_CURRENT_SOURCE_DIR}/src/getopt/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/src/getopt/*.c
${CMAKE_CURRENT_SOURCE_DIR}/src/getopt/*.h)
file(GLOB PROJECT_SRC_MESA
${CMAKE_CURRENT_SOURCE_DIR}/src/mesa/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/src/mesa/*.c
${CMAKE_CURRENT_SOURCE_DIR}/src/mesa/*.h)
file(GLOB PROJECT_SRC_UTIL
${CMAKE_CURRENT_SOURCE_DIR}/src/util/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/src/util/*.c
${CMAKE_CURRENT_SOURCE_DIR}/src/util/*.h)
file(GLOB PROJECT_SRC_COMPILER_GLSL
${CMAKE_CURRENT_SOURCE_DIR}/src/compiler/glsl/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/src/compiler/glsl/*.c
${CMAKE_CURRENT_SOURCE_DIR}/src/compiler/glsl/*.h)
file(GLOB PROJECT_SRC_COMPILER_GLSL_GLCPP
${CMAKE_CURRENT_SOURCE_DIR}/src/compiler/glsl/glcpp/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/src/compiler/glsl/glcpp/*.c
${CMAKE_CURRENT_SOURCE_DIR}/src/compiler/glsl/glcpp/*.h)
file(GLOB PROJECT_SRC_MESA_MAIN
${CMAKE_CURRENT_SOURCE_DIR}/src/mesa/main/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/src/mesa/main/*.c
${CMAKE_CURRENT_SOURCE_DIR}/src/mesa/main/*.h)
file(GLOB PROJECT_SRC_MESA_PROGRAM
${CMAKE_CURRENT_SOURCE_DIR}/src/mesa/program/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/src/mesa/program/*.c
${CMAKE_CURRENT_SOURCE_DIR}/src/mesa/program/*.h)
file(GLOB PROJECT_SRC_UTIL_SHA1
${CMAKE_CURRENT_SOURCE_DIR}/src/util/sha1/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/src/util/sha1/*.c
${CMAKE_CURRENT_SOURCE_DIR}/src/util/sha1/*.h)
file(GLOB PROJECT_SRC_MESA_MATH
${CMAKE_CURRENT_SOURCE_DIR}/src/mesa/math/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/src/mesa/math/*.c
${CMAKE_CURRENT_SOURCE_DIR}/src/mesa/math/*.h)
file(GLOB PROJECT_SRC_MESA_VBO
${CMAKE_CURRENT_SOURCE_DIR}/src/mesa/vbo/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/src/mesa/vbo/*.c
${CMAKE_CURRENT_SOURCE_DIR}/src/mesa/vbo/*.h)
file(GLOB PROJECT_SRC_GALLIUM
${CMAKE_CURRENT_SOURCE_DIR}/src/gallium/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/src/gallium/*.c
${CMAKE_CURRENT_SOURCE_DIR}/src/gallium/*.h)
file(GLOB PROJECT_SRC_GALLIUM_AUXILIARY
${CMAKE_CURRENT_SOURCE_DIR}/src/gallium/auxiliary/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/src/gallium/auxiliary/*.c
${CMAKE_CURRENT_SOURCE_DIR}/src/gallium/auxiliary/*.h)
file(GLOB PROJECT_SRC_GALLIUM_AUXILIARY_UTIL
${CMAKE_CURRENT_SOURCE_DIR}/src/gallium/auxiliary/util/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/src/gallium/auxiliary/util/*.c
${CMAKE_CURRENT_SOURCE_DIR}/src/gallium/auxiliary/util/*.h)
file(GLOB PROJECT_SRC_GALLIUM_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/src/gallium/include/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/src/gallium/include/*.c
${CMAKE_CURRENT_SOURCE_DIR}/src/gallium/include/*.h)
file(GLOB PROJECT_SRC_GALLIUM_INCLUDE_PIPE
${CMAKE_CURRENT_SOURCE_DIR}/src/gallium/include/pipe/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/src/gallium/include/pipe/*.c
${CMAKE_CURRENT_SOURCE_DIR}/src/gallium/include/pipe/*.h)
file(GLOB PROJECT_SRC_MAPI
${CMAKE_CURRENT_SOURCE_DIR}/src/mapi/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/src/mapi/*.c
${CMAKE_CURRENT_SOURCE_DIR}/src/mapi/*.h)
file(GLOB PROJECT_SRC_MAPI_GLAPI
${CMAKE_CURRENT_SOURCE_DIR}/src/mapi/glapi/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/src/mapi/glapi/*.c
${CMAKE_CURRENT_SOURCE_DIR}/src/mapi/glapi/*.h)
file(GLOB PROJECT_SRC_CODE
${CMAKE_CURRENT_SOURCE_DIR}/src/code/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/src/code/*.c
${CMAKE_CURRENT_SOURCE_DIR}/src/code/*.h)
file(GLOB PROJECT_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/include/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/include/*.c
${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB PROJECT_INCLUDE_GL
${CMAKE_CURRENT_SOURCE_DIR}/include/GL/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/include/GL/*.c
${CMAKE_CURRENT_SOURCE_DIR}/include/GL/*.h)
file(GLOB PROJECT_INCLUDE_C11
${CMAKE_CURRENT_SOURCE_DIR}/include/c11/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/include/c11/*.c
${CMAKE_CURRENT_SOURCE_DIR}/include/c11/*.h)
source_group(src FILES ${PROJECT_SRC})
source_group(src\\compiler FILES ${PROJECT_SRC_COMPILER})
source_group(src\\getopt FILES ${PROJECT_SRC_GETOPT})
source_group(src\\mesa FILES ${PROJECT_SRC_MESA})
source_group(src\\util FILES ${PROJECT_SRC_UTIL})
source_group(src\\compiler\\glsl FILES ${PROJECT_SRC_COMPILER_GLSL})
source_group(src\\compiler\\glsl\\glcpp FILES ${PROJECT_SRC_COMPILER_GLSL_GLCPP})
source_group(src\\mesa\\main FILES ${PROJECT_SRC_MESA_MAIN})
source_group(src\\mesa\\program FILES ${PROJECT_SRC_MESA_PROGRAM})
source_group(src\\util\\sha1 FILES ${PROJECT_SRC_UTIL_SHA1})
source_group(src\\mesa\\math FILES ${PROJECT_SRC_MESA_MATH})
source_group(src\\mesa\\vbo FILES ${PROJECT_SRC_MESA_VBO})
source_group(src\\gallium FILES ${PROJECT_SRC_GALLIUM})
source_group(src\\gallium\\auxiliary FILES ${PROJECT_SRC_GALLIUM_AUXILIARY})
source_group(src\\gallium\\auxiliary\\util FILES ${PROJECT_SRC_GALLIUM_AUXILIARY_UTIL})
source_group(src\\gallium\\include FILES ${PROJECT_SRC_GALLIUM_INCLUDE})
source_group(src\\gallium\\include\\pipe FILES ${PROJECT_SRC_GALLIUM_INCLUDE_PIPE})
source_group(src\\mapi FILES ${PROJECT_SRC_MAPI})
source_group(src\\mapi\\glapi FILES ${PROJECT_SRC_MAPI_GLAPI})
source_group(src\\code FILES ${PROJECT_SRC_CODE})
source_group(include FILES ${PROJECT_INCLUDE})
source_group(include\\GL FILES ${PROJECT_INCLUDE_GL})
source_group(include\\c11 FILES ${PROJECT_INCLUDE_C11})

add_definitions(-D_LIB)
add_definitions(-DNOMINMAX)
add_definitions(-D__STDC_VERSION__=199901L)
add_definitions(-D__STDC_LIMIT_MACROS)
add_definitions(-D__STDC_FORMAT_MACROS)
add_definitions(-D__STDC_CONSTANT_MACROS)
add_definitions(-D__STDC_WANT_LIB_EXT2__) ## strdup
add_definitions(-D__STDC_NO_THREADS__)
add_definitions(-D_USE_MATH_DEFINES)

##add_definitions(-Dstricmp=_stricmp)


if(UNIX)
	if(APPLE)
		add_definitions(-DAPPLE)
	else()
		add_definitions(-DLINUX)
	endif()
	add_definitions(-DUNIX)
elseif(WIN32)
	add_definitions(-DWIN32)
	if(MINGW)
		add_definitions(-DMINGW32)
		add_definitions(-DHAVE_STRTOK_R)
		add_definitions(-DHAVE___BUILTIN_FFS)
	endif()
endif()

if(MSVC)
    # Ignore 4055 for glad
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /IGNORE:4055")
	add_definitions(-D__STDC__)
	add_definitions(-DMSVC)
	add_definitions(-Dstrdup=_strdup)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra") ## -std=gnu++0x
endif ()

add_library(GlslOptimizerV2 STATIC 
	${PROJECT_SRC}
	${PROJECT_SRC_COMPILER}
	${PROJECT_SRC_GETOPT}
	${PROJECT_SRC_MESA}
	${PROJECT_SRC_UTIL}
	${PROJECT_SRC_COMPILER_GLSL}
	${PROJECT_SRC_COMPILER_GLSL_GLCPP}
	${PROJECT_SRC_MESA_MAIN}
	${PROJECT_SRC_MESA_PROGRAM}
	${PROJECT_SRC_UTIL_SHA1}
	${PROJECT_SRC_MESA_MATH}
	${PROJECT_SRC_MESA_VBO}
	${PROJECT_SRC_GALLIUM}
	${PROJECT_SRC_GALLIUM_AUXILIARY}
	${PROJECT_SRC_GALLIUM_AUXILIARY_UTIL}
	${PROJECT_SRC_GALLIUM_INCLUDE}
	${PROJECT_SRC_GALLIUM_INCLUDE_PIPE}
	${PROJECT_SRC_MAPI}
	${PROJECT_SRC_MAPI_GLAPI}
	${PROJECT_SRC_CODE}
	${PROJECT_INCLUDE}
	${PROJECT_INCLUDE_GL}
	${PROJECT_INCLUDE_C11}
)
set_target_properties(GlslOptimizerV2 PROPERTIES LINKER_LANGUAGE CXX)

include_directories(
		src
		src/mesa
		include
		include/c99
		include/c11
		src/compiler
		src/compiler/glsl
		src/mapi
		src/getopt
		src/gallium/include
		src/gallium/auxiliary
		src/util
		../../src/gallium/auxiliary
		../../src/util
)

set(GLSLOPTIMIZER_INCLUDE_DIR 
	${CMAKE_CURRENT_SOURCE_DIR} 
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/src/mesa
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/include/c99
	${CMAKE_CURRENT_SOURCE_DIR}/include/c11
	${CMAKE_CURRENT_SOURCE_DIR}/src/compiler
	${CMAKE_CURRENT_SOURCE_DIR}/src/compiler/glsl
	${CMAKE_CURRENT_SOURCE_DIR}/src/mapi
	${CMAKE_CURRENT_SOURCE_DIR}/src/getopt
	${CMAKE_CURRENT_SOURCE_DIR}/src/gallium/include
	${CMAKE_CURRENT_SOURCE_DIR}/src/gallium/auxiliary
	${CMAKE_CURRENT_SOURCE_DIR}/src/util
	PARENT_SCOPE)

set(GLSLOPTIMIZER_LIBRARIES GlslOptimizerV2 PARENT_SCOPE)
set(GLSLOPTIMIZER_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)
