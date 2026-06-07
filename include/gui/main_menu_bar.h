#pragma once

#include <string>

#include "imgui_wrapper.h"

class Application;

namespace gui {

class GuiManager;

class MainMenuBar {
    Application& app;
    GuiManager& gui_manager_;

    bool is_saving = false;
    std::string layout_name_input{};

public:
    explicit MainMenuBar(Application& app, GuiManager& gui_manager)
        : app{app}, gui_manager_{gui_manager} {}

    void render_gui();

private:
    void render_input_layout_modal();
};

} // namespace gui
