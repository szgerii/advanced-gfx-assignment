#include "gui/panels/general_config.h"

#include "app/application.h"
#include "render/render_command.h"
#include "utils/num_cast.h"

#include <string_view>
#include <vector>

namespace gui {

void GeneralConfigPanel::render_panel() {
    const auto scenes    = app_.scenes();
    const auto pipelines = app_.pipelines();

    if (ImGui::BeginListBox("Scene selector")) {
        const bool no_scene_active = app_.active_scene_idx() < 0;

        if (ImGui::Selectable("None", no_scene_active))
            app_.switch_scene(-1);

        if (no_scene_active)
            ImGui::SetItemDefaultFocus();

        for (size_t i = 0; i < scenes.size(); i++) {
            const bool is_selected = app_.active_scene_idx() == num_cast<int>(i);

            if (ImGui::Selectable(scenes[i]->name.c_str(), is_selected))
                app_.switch_scene(num_cast<int>(i));

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndListBox();
    }

    if (ImGui::BeginListBox("Pipeline selector")) {
        const bool no_pipeline_active = app_.active_pipeline_idx() < 0;

        if (ImGui::Selectable("None", no_pipeline_active))
            app_.switch_pipeline(-1);

        if (no_pipeline_active)
            ImGui::SetItemDefaultFocus();

        for (size_t i = 0; i < pipelines.size(); i++) {
            const bool is_selected = app_.active_pipeline_idx() == num_cast<int>(i);

            if (ImGui::Selectable(pipelines[i]->name.c_str(), is_selected))
                app_.switch_pipeline(num_cast<int>(i));

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndListBox();
    }

    ImGui::SeparatorText("Display Mode");

#ifndef NDEBUG
    static bool title_app_name_ = true;
    static bool title_ogl_      = true;
    static bool title_glfw_     = true;
#else
    static bool title_app_name_ = true;
    static bool title_ogl_      = false;
    static bool title_glfw_     = false;
#endif

    static int pending_mode_idx_ = -1;

    auto& props                   = app_.window_.properties;
    bool instant_update_triggered = false;

    if (pending_mode_idx_ == -1)
        pending_mode_idx_ = static_cast<int>(props.mode);

    const char* mode_names[] = {"FullScreen", "WindowedFullScreen", "Windowed"};

    ImGui::Combo("Window Mode", &pending_mode_idx_, mode_names, IM_ARRAYSIZE(mode_names));

    bool mode_changed = (pending_mode_idx_ != static_cast<int>(props.mode));
    ImGui::BeginDisabled(!mode_changed);
    if (ImGui::Button("Apply Mode")) {
        props.mode               = static_cast<WindowMode>(pending_mode_idx_);
        instant_update_triggered = true;
    }
    ImGui::EndDisabled();

    ImGui::SeparatorText("Resolution");

    bool is_fullscreen = (props.mode != WindowMode::Windowed);
    ImGui::BeginDisabled(is_fullscreen);

    instant_update_triggered |= ImGui::DragInt("Width", &props.width, 5.0f, 640, 7680);
    instant_update_triggered |= ImGui::DragInt("Height", &props.height, 5.0f, 480, 4320);
    instant_update_triggered |= ImGui::Checkbox("Resizable", &props.resizable);

    ImGui::EndDisabled();

    if (is_fullscreen)
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                           "Resolution is locked by fullscreen mode.");

    ImGui::SeparatorText("Rendering");
    instant_update_triggered |= ImGui::Checkbox("VSync", &props.vsync);

    if (instant_update_triggered)
        app_.window_.sync_properties();

    ImGui::SeparatorText("Window Title");

    static bool use_auto_title = false;
    static std::string last_custom_title{};

    bool old_use_auto_title = use_auto_title;
    bool title_changed      = false;
    title_changed |= ImGui::Checkbox("Use Automatic Window Name", &use_auto_title);

    if (use_auto_title && !old_use_auto_title)
        last_custom_title = app_.window_.properties.title;

    ImGui::BeginDisabled(!use_auto_title);
    title_changed |= ImGui::Checkbox("Include Application Name", &title_app_name_);
    title_changed |= ImGui::Checkbox("Include OpenGL Version", &title_ogl_);
    title_changed |= ImGui::Checkbox("Include GLFW Version", &title_glfw_);
    ImGui::EndDisabled();

    if (title_changed)
        app_.window_.set_info_title(title_app_name_, title_ogl_, title_glfw_);

    if (!use_auto_title && old_use_auto_title) {
        app_.window_.properties.title = last_custom_title;
        app_.window_.sync_properties();
    }
}

} // namespace gui