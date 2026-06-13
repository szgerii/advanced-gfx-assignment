#pragma once

#include "gui/panel.h"

class IApplication;

namespace gui {

class ImGuiConfigPanel final : public IPanel {
    IApplication& app_;
    bool show_demo_window_ = false;

public:
    explicit ImGuiConfigPanel(IApplication& app)
        : IPanel{"ImGui"}, app_{app} {}

protected:
    void render_gui() override {
        IPanel::render_gui();

        if (show_demo_window_)
            ImGui::ShowDemoWindow(&show_demo_window_);
    }

    void render_panel() override;
};

} // namespace gui
