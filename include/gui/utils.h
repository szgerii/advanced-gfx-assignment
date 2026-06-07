#pragma once

#include "imgui_wrapper.h"

namespace gui {

class ImGuiColorRAII {
public:
    explicit ImGuiColorRAII(ImGuiCol target, ImU32 col) { ImGui::PushStyleColor(target, col); }

    explicit ImGuiColorRAII(ImGuiCol target, const ImVec4& col) {
        ImGui::PushStyleColor(target, col);
    }

    ~ImGuiColorRAII() { ImGui::PopStyleColor(); }

    ImGuiColorRAII(const ImGuiColorRAII&)            = delete;
    ImGuiColorRAII& operator=(const ImGuiColorRAII&) = delete;
    ImGuiColorRAII(ImGuiColorRAII&&)                 = delete;
    ImGuiColorRAII& operator=(ImGuiColorRAII&&)      = delete;
};

class ImGuiFontRAII {
public:
    explicit ImGuiFontRAII(ImFont* font) { ImGui::PushFont(font); }
    ~ImGuiFontRAII() { ImGui::PopFont(); }

    ImGuiFontRAII(const ImGuiFontRAII&)            = delete;
    ImGuiFontRAII& operator=(const ImGuiFontRAII&) = delete;
    ImGuiFontRAII(ImGuiFontRAII&&)                 = delete;
    ImGuiFontRAII& operator=(ImGuiFontRAII&&)      = delete;
};

class ImGuiDisabledRAII {
public:
    explicit ImGuiDisabledRAII(bool disabled = true) { ImGui::BeginDisabled(disabled); }
    ~ImGuiDisabledRAII() { ImGui::EndDisabled(); }

    ImGuiDisabledRAII(const ImGuiDisabledRAII&)            = delete;
    ImGuiDisabledRAII& operator=(const ImGuiDisabledRAII&) = delete;
    ImGuiDisabledRAII(ImGuiDisabledRAII&&)                 = delete;
    ImGuiDisabledRAII& operator=(ImGuiDisabledRAII&&)      = delete;
};

} // namespace gui