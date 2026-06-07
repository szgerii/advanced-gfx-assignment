#include "gui/panels/imgui_config.h"

#include "app/application.h"
#include "gui/layout_manager.h"

namespace gui {

void ImGuiConfigPanel::render_panel() {
    ImGui::SeparatorText("Available Layouts");

    const auto& avail_layouts =
        LayoutManager::get_layout_names(app_.current_update_info().total_ms);

    if (ImGui::BeginListBox("##Layout selector")) {
        for (const auto& layout : avail_layouts)
            if (ImGui::Selectable(layout.c_str(), false))
                LayoutManager::try_load_layout(layout);

        ImGui::EndListBox();
    }

    ImGui::SeparatorText("Misc");

    ImGui::Checkbox("Show ImGui demo window", &show_demo_window_);
}

} // namespace gui
