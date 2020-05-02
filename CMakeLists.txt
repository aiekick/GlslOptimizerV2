cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0012 NEW)
cmake_policy(SET CMP0023 NEW)

set(PROJECT GlslOptimizer)

enable_language(C CXX)

project(${PROJECT} CXX)

include(cmake/3rdparty.cmake)

## some defines for debug mode
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
add_definitions(-DPROJECT_PATH="${CMAKE_SOURCE_DIR}")

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
	endif()
    set(DIRENT_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/dirent/include)
    include_directories(${DIRENT_INCLUDE_DIR})
endif()

if(MSVC)
    # Ignore 4055 for glad
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /IGNORE:4055")
    add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-DMSVC)
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++11")
endif ()

add_definitions(${GLFW_DEFINITIONS})
add_definitions(-DCUSTOM_IMGUIFILEDIALOG_CONFIG="../../../src/Gui/CustomImGuiFileDialogConfig.h")

file(GLOB PROJECT_SOURCES ${CMAKE_SOURCE_DIR}/*.cpp)
file(GLOB PROJECT_SRC 
		${CMAKE_SOURCE_DIR}/src/*.cpp 
		${CMAKE_SOURCE_DIR}/src/*.h) 
file(GLOB PROJECT_HELPER 
		${CMAKE_SOURCE_DIR}/src/Helper/*.cpp 
		${CMAKE_SOURCE_DIR}/src/Helper/*.h)
file(GLOB PROJECT_IMGUIIMPL
		${CMAKE_SOURCE_DIR}/src/ImguiImpl/*.cpp 
		${CMAKE_SOURCE_DIR}/src/ImguiImpl/*.h)
file(GLOB PROJECT_RES
		${CMAKE_SOURCE_DIR}/src/Res/*.cpp 
		${CMAKE_SOURCE_DIR}/src/Res/*.h)
file(GLOB PROJECT_GUI
		${CMAKE_SOURCE_DIR}/src/Gui/*.cpp 
		${CMAKE_SOURCE_DIR}/src/Gui/*.h)
file(GLOB PROJECT_PROJECT
		${CMAKE_SOURCE_DIR}/src/Project/*.cpp 
		${CMAKE_SOURCE_DIR}/src/Project/*.h)		
file(GLOB PROJECT_PANES
		${CMAKE_SOURCE_DIR}/src/Panes/*.cpp 
		${CMAKE_SOURCE_DIR}/src/Panes/*.h)
source_group(main FILES ${PROJECT_SOURCES})
source_group(src FILES ${PROJECT_SRC})
source_group(src\\Helper FILES ${PROJECT_HELPER})
source_group(src\\ImguiImpl FILES ${PROJECT_IMGUIIMPL})
source_group(src\\Panes FILES ${PROJECT_PANES})
source_group(src\\Project FILES ${PROJECT_PROJECT})
source_group(src\\Res FILES ${PROJECT_RES})
source_group(src\\Gui FILES ${PROJECT_GUI})

if (APPLE)
	##set(ICON icon.icns)
	##set_source_files_properties(icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

add_executable(${PROJECT}
		MACOSX_BUNDLE
		##${ICON}
		${PROJECT_SOURCES} 
		${PROJECT_SRC} 
		${PROJECT_IMGUIFILEDIALOG} 
		${PROJECT_HELPER}
		${PROJECT_IMGUIIMPL}
		${PROJECT_GENERATOR}
		${PROJECT_GUI}
		${PROJECT_RES}
		${PROJECT_PROJECT}
		${PROJECT_PANES}
)

if (APPLE)
	set_target_properties(${PROJECT}
			PROPERTIES
			OUTPUT_NAME "GlslOptimizer_Osx"
			MACOSX_BUNDLE_EXECUTABLE_NAME "GlslOptimizer_Osx"
			MACOSX_BUNDLE_BUNDLE_NAME "GlslOptimizer_Osx"
			FOLDER "GlslOptimizer_Osx"
			##RESOURCE icon.icns
			MACOSX_BUNDLE_SHORT_VERSION_STRING "0.5"
			MACOSX_BUNDLE_LONG_VERSION_STRING "Beta 0.5"
			##MACOSX_BUNDLE_ICON_FILE icon.icns
			MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/MacOSXBundleInfo.plist.in")
elseif(WIN32)
	if (MINGW)
		set_target_properties(${PROJECT} PROPERTIES	OUTPUT_NAME "GlslOptimizer_Mingw32_Win32")
	elseif (CYGWIN)
		set_target_properties(${PROJECT} PROPERTIES	OUTPUT_NAME "GlslOptimizer_Cygwin_Win32")
	elseif(MSVC)
		set_target_properties(${PROJECT} PROPERTIES	OUTPUT_NAME "GlslOptimizer_Msvc_Win32")
	endif()
elseif(UNIX)
	set_target_properties(${PROJECT} PROPERTIES	OUTPUT_NAME "GlslOptimizer_Linux")
endif()

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}")
install(TARGETS ${PROJECT} DESTINATION ${CMAKE_INSTALL_PREFIX})

include_directories(
		src
        ${PLOG_INCLUDE_DIR}
        ${OPENGL_INCLUDE_DIR}
        ${GLFW_INCLUDE_DIR}
        ${GLAD_INCLUDE_DIR}
        ${IMGUI_INCLUDE_DIR}
		${CTOOLS_INCLUDE_DIR}
		${TINYXML2_INCLUDE_DIR}
		${IMGUIFILEDIALOG_INCLUDE_DIR}
)

target_link_libraries(${PROJECT} PRIVATE
        ${OPENGL_LIBRARIES}
        ${GLFW_LIBRARIES}
        ${GLAD_LIBRARIES}
        ${IMGUI_LIBRARIES}
        ${CTOOLS_LIBRARIES}
		${TINYXML2_LIBRARIES}
		${IMGUIFILEDIALOG_LIBRARIES}
)
