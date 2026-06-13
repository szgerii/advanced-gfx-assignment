#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

static_assert(IMGUI_VERSION_NUM >= 19200, "expected ImGui version to be at least 1.92");

#ifndef IMGUI_HAS_DOCK
    #error Expected ImGui to have docking feature
#endif

#ifndef IMGUI_HAS_VIEWPORT
    #error Expected ImGui to have viewports feature
#endif
