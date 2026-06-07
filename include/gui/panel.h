#pragma once

#include <string>

#include "imgui_wrapper.h"

namespace gui {

class IPanel {
public:
    static constexpr ImGuiWindowFlags DEFAULT_PANEL_WINDOW_FLAGS =
        ImGuiWindowFlags_NoFocusOnAppearing;

    std::string name;
    ImGuiWindowFlags window_flags;

    explicit IPanel(std::string name, ImGuiWindowFlags window_flags = DEFAULT_PANEL_WINDOW_FLAGS)
        : name{std::move(name)}, window_flags{window_flags} {}

    virtual ~IPanel() = default;

    bool is_open = true;

    virtual void render_gui() {
        if (!is_open)
            return;

        ImGui::Begin(name.c_str(), &is_open, window_flags);
        render_panel();
        ImGui::End();
    }

protected:
    virtual void render_panel() = 0;
};

} // namespace gui
