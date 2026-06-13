#pragma once

#include <format>
#include <string>
#include <string_view>

#include "gui/panel.h"
#include "gui/styles.h"

class IScene;

namespace gui {

class ScenePanel : public IPanel {
private:
    IScene& scene_;

public:
    explicit ScenePanel(IScene& scene, ImGuiWindowFlags window_flags = DEFAULT_PANEL_WINDOW_FLAGS);

    virtual void render_gui() override final {
        if (!is_open)
            return;

        ImGui::Begin("Active Scene", &is_open, window_flags);

        render_base();
        render_panel();

        ImGui::End();
    }

protected:
    virtual void render_panel() override {
        ImGui::SeparatorText("Scene Config");
        ImGui::Text("This scene does not provide any configurations to edit");
    }

    void render_base();
};

} // namespace gui
