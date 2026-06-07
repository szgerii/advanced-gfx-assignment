#include "gui/main_menu_bar.h"

#include <string>
#include <string_view>

#include "app/application.h"
#include "gui/gui_manager.h"
#include "gui/layout_manager.h"
#include "misc/cpp/imgui_stdlib.h"
#include "utils/enum_range.h"

namespace gui {

void MainMenuBar::render_gui() {
    bool do_open_layout_input = false;

    static bool menu_bar_open_before = false;

    if (ImGui::BeginMainMenuBar()) {
        // afaik, there's no reason to avoid BeginMainMenuBar stealing focus on appear, without
        // reimplementing/modifying internals (unfortunate)
        if (!menu_bar_open_before) {
            // since we should only be transitioning from a no-gui state into having a menu bar
            // appear, we don't have to worry about another window already having focus before the
            // menu bar is focused during BeginMainMenuBar()
            menu_bar_open_before = true;
            ImGui::SetWindowFocus(nullptr);
        }

        if (ImGui::BeginMenu("File")) {
            ImGui::Separator();

            // layout persistence
            if (ImGui::MenuItem("Save current layout")) {
                layout_name_input.clear();
                is_saving            = true;
                do_open_layout_input = true;
            }

            if (ImGui::MenuItem("Load layout from path")) {
                layout_name_input.clear();
                is_saving            = false;
                do_open_layout_input = true;
            }

            // only refresh layout list on submenu open, not subsequent draws
            static bool force_refresh_layout_list = true;
            if (ImGui::BeginMenu("Load layout")) {
                // make async
                const auto& layouts = LayoutManager::get_layout_names(
                    app.current_update_info().total_ms, force_refresh_layout_list);
                if (force_refresh_layout_list)
                    force_refresh_layout_list = false;

                if (layouts.empty()) {
                    ImGuiDisabledRAII{};
                    ImGui::Text("No layouts are available");
                }

                for (std::string_view layout : layouts) {
                    std::string name{std::format("{}##Layout Menu Item", layout)};
                    if (ImGui::MenuItem(name.c_str()))
                        LayoutManager::try_load_layout(layout);
                }

                ImGui::EndMenu();
            } else {
                // force layout list refresh on next open
                force_refresh_layout_list = true;
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Quit", "Alt+F4"))
                app.signal_quit();

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            for (auto it : enum_range<GuiShowState>()) {
                std::string label_str{gui_show_state_str(it)};
                if (ImGui::RadioButton(label_str.c_str(), gui_manager_.gui_show_state == it))
                    gui_manager_.gui_show_state = it;
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (do_open_layout_input)
        ImGui::OpenPopup("Input layout name");

    render_input_layout_modal();
}

void MainMenuBar::render_input_layout_modal() {
    if (ImGui::BeginPopupModal("Input layout name", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Layout name: ");

        if (ImGui::IsWindowAppearing())
            ImGui::SetKeyboardFocusHere();

        bool entered = ImGui::InputText("##Layout name", &layout_name_input,
                                        ImGuiInputTextFlags_EnterReturnsTrue);

        if (ImGui::Button("OK", ImVec2(120, 0)) || entered) {
            if (!layout_name_input.empty()) {
                if (is_saving)
                    LayoutManager::try_save_layout_as(layout_name_input);
                else
                    LayoutManager::try_load_layout(layout_name_input);

                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }
}

} // namespace gui
