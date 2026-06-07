#pragma once

#include <utility>

#include "gui/utils.h"

namespace gui {

template <typename... FmtArgs>
inline void Header(const char* txt, FmtArgs&&... fmt_args) {
    ImGuiColorRAII{ImGuiCol_Text, IM_COL32(100, 210, 255, 255)};

    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::Text(txt, std::forward<FmtArgs>(fmt_args)...);

    ImGui::Separator();
    ImGui::Spacing();
}

} // namespace gui
