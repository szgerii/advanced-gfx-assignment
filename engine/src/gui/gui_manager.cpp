#include "gui/gui_manager.h"

#include "core/application.h"
#include "core/meta.h"
#include "utils/enum_range.h"

namespace gui {

void GuiManager::init() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    io.ConfigDpiScaleFonts     = true;
    io.ConfigDpiScaleViewports = true;

    // TODO
    // io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;
    // io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;
    // io.ConfigViewportsNoAutoMerge = true;
    // io.ConfigViewportsNoTaskBarIcon = true;

    io.Fonts->AddFontFromFileTTF(meta::get_font_path("JetBrainsMono-Regular").string().c_str());

    ImGui::StyleColorsDark();

    float main_scale   = ImGui_ImplGlfw_GetContentScaleForWindow(app_.window_.get_handle());
    ImGuiStyle& style  = ImGui::GetStyle();
    style.FontSizeBase = 16;
    style.FontScaleDpi = main_scale;

    style.ScaleAllSizes(main_scale); // TODO: this is single-viewport-only

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding              = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplGlfw_InitForOpenGL(app_.window_.get_handle(), true);
    ImGui_ImplOpenGL3_Init("#version 460"); // TODO: centralize this info

    initialized_ = true;
}

void GuiManager::update([[maybe_unused]] const UpdateInfo& update_info) {
    const auto& key_evs = app_.input_manager().key_events();
    for (KeyEvent ev : key_evs) {
        if (ev.action != GLFW_PRESS)
            continue;

        if (!(ev.mods & GLFW_MOD_CONTROL && ev.mods & GLFW_MOD_ALT))
            continue;

        // maps CTRL+ALT+[1-9] to gui states 0 to min(gui state count, 9)
        // i dont wanna come back and manually add shortcuts for further gui modes later

        static_assert(GLFW_KEY_9 - GLFW_KEY_1 == 8,
                      "glfw num key enums must be continous for the below code to work");
        static_assert(enum_count<GuiShowState>() <= 9);

        std::underlying_type_t<GuiShowState> i = 0;
        for (auto it : enum_range<GuiShowState>()) {
            if (ev.key == GLFW_KEY_1 + i)
                gui_show_state = it;

            i++;
        }
    }
}

void GuiManager::render_gui() {

    begin_frame();

    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(),
                                 ImGuiDockNodeFlags_PassthruCentralNode);

    if (gui_show_state != GuiShowState::Hidden) {
        main_menu_bar_.render_gui();

        if (gui_show_state == GuiShowState::Full) {
            for (auto& panel : panels_)
                panel->render_gui();

            if (app_.active_scene_idx() >= 0) {
                auto& scene = app_.active_scene();

                scene.render_scene_panel();
                scene.render_gui();
            }
        }
    }

    end_frame();
}

} // namespace gui
