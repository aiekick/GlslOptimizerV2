// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "imgui.h"
#include "src/ImguiImpl/imgui_impl_glfw.h"
#include "src/ImguiImpl/imgui_impl_opengl3.h"
#include <stdio.h>

#include <FileHelper.h>

#include "ctools/GLVersionChecker.h"
#include "src/MainFrame.h"
#include "Res/CustomFont.cpp"

#include <glad/glad.h> 
#include <GLFW/glfw3.h>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

static void glfw_window_close_callback(GLFWwindow* window)
{
	glfwSetWindowShouldClose(window, GL_FALSE); // block app closing
	MainFrame::Instance()->IWantToCloseTheApp();
}

bool IsGlversionSupported(int vMajor, int vMinor)
{
	bool res = false;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, vMajor);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, vMinor);
	glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
	auto hnd = glfwCreateWindow(1, 1, "", NULL, NULL);
	if (hnd)
	{
		res = true;
		glfwDestroyWindow(hnd);
	}

	return res;
}

void FoundBestOpenGLVersionAvailable(int *vMajor, int *vMinor)
{
	// 4.5
	if (IsGlversionSupported(4, 5)) { *vMajor = 4; *vMinor = 5; return; }
	// 4.4
	if (IsGlversionSupported(4, 4)) { *vMajor = 4; *vMinor = 4; return; }
	// 4.3 => compute
	if (IsGlversionSupported(4, 3)) { *vMajor = 4; *vMinor = 3; return; }
	// 4.2
	if (IsGlversionSupported(4, 2)) { *vMajor = 4; *vMinor = 2; return; }
	// 4.1
	if (IsGlversionSupported(4, 1)) { *vMajor = 4; *vMinor = 1; return; }
	// 4.0 => tesselation
	if (IsGlversionSupported(4, 0)) { *vMajor = 4; *vMinor = 0; return; }
	// 3.3 => attrcib lcoation in core
	if (IsGlversionSupported(3, 3)) { *vMajor = 3; *vMinor = 3; return; }
	// 3.2 => attrib location in extention
	if (IsGlversionSupported(3, 2)) { *vMajor = 3; *vMinor = 2; return; }
	// on charge que les cores. ya pas de core avant 3.2
}

int main(int, char**argv)
{
	FileHelper::Instance()->SetAppPath(std::string(argv[0]));
#ifndef _DEBUG
	FileHelper::Instance()->SetCurDirectory(FileHelper::Instance()->GetAppPath());
#endif

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

	// Decide GL+GLSL versions
#if APPLE
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

	// version 4.3 for support of compute shaders
	int opengl_Major = 0;
	int opengl_Minor = 0;

	// minimum version demand�e, mais glfw peut en cree une superieure
	FoundBestOpenGLVersionAvailable(&opengl_Major, &opengl_Minor);

	// base version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, opengl_Major);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, opengl_Minor);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window with graphics context
    GLFWwindow* mainWindow = glfwCreateWindow(1280, 720, "GlslOptimizer", 0, 0);
    if (mainWindow == 0)
        return 1;

    glfwMakeContextCurrent(mainWindow);
    glfwSwapInterval(1); // Enable vsync
	glfwSetWindowCloseCallback(mainWindow, glfw_window_close_callback);

    if (gladLoadGL() == 0)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

	std::string glslVersion = GLVersionChecker::Instance()->GetGlslVersionHeader();

#ifdef MSVC
	//#ifndef _DEBUG
		ShowWindow(GetConsoleWindow(), SW_HIDE); // hide console
	//#endif
#endif

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.FontAllowUserScaling = true; // activate zoom feature with ctrl + mousewheel

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(mainWindow, true);
    ImGui_ImplOpenGL3_Init(glslVersion.c_str());

	// load memory font file
	ImGui::GetIO().Fonts->AddFontDefault();
	static const ImWchar icons_ranges[] = { ICON_MIN_IGFS, ICON_MAX_IGFS, 0 };
	ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
	ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_IGFS, 15.0f, &icons_config, icons_ranges);
   
	MainFrame::Instance(mainWindow)->Init();

    // Main loop
	int display_w, display_h;
	while (!glfwWindowShouldClose(mainWindow))
    {
		glfwGetFramebufferSize(mainWindow, &display_w, &display_h);
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
		
		MainFrame::Instance()->Display(ImVec2((float)display_w, (float)display_h));

        ImGui::Render();
        glViewport(0, 0, display_w, display_h);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(mainWindow);
    }

	MainFrame::Instance()->Unit();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(mainWindow);
    glfwTerminate();

    return 0;
}
