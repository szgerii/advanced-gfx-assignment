#pragma once

#include <string>

#include "gui/imgui_wrapper.h"

class IApplication;

namespace gui {

class GuiManager;

class MainMenuBar {
    IApplication& app;
    GuiManager& gui_manager_;

    bool is_saving = false;
    std::string layout_name_input{};

public:
    explicit MainMenuBar(IApplication& app, GuiManager& gui_manager)
        : app{app}, gui_manager_{gui_manager} {}

    void render_gui();

private:
    void render_input_layout_modal();
};

} // namespace gui
