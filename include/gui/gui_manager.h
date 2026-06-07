#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

#include "app/common.h"
#include "app/window.h"
#include "gui/main_menu_bar.h"
#include "gui/panel.h"
#include "gui/scene_panel.h"
#include "vendor.h"

class Application;

namespace gui {

enum class GuiShowState : uint8_t { FIRST = 0, Full = FIRST, MenuOnly, Hidden, LAST = Hidden };

inline constexpr std::string_view gui_show_state_str(GuiShowState state) {
    switch (state) {
        case GuiShowState::Hidden:
            return "Hidden";
        case GuiShowState::MenuOnly:
            return "Menu Bar Only";
        case GuiShowState::Full:
            return "Full";
        default:
            throw std::logic_error{"missing GuiShowState case"};
    }
}

class GuiManager {
    Application& app_;
    MainMenuBar main_menu_bar_;
    std::vector<std::unique_ptr<IPanel>> panels_{};

public:
    GuiShowState gui_show_state = GuiShowState::Full;

    explicit GuiManager(Application& app, bool do_init)
        : app_{app}, main_menu_bar_{app, *this} {
        if (do_init)
            init();
    }

    ~GuiManager() { cleanup(); }

    void init();

    void attach_panel(std::unique_ptr<IPanel> panel) { panels_.emplace_back(std::move(panel)); }

    void begin_frame() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void update(const UpdateInfo& update_info);

    void render_gui();

    void end_frame() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }

private:
    bool initialized_ = false;

    void cleanup() {
        if (!initialized_)
            return;

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
};

}; // namespace gui
