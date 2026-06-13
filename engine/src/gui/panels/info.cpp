#include "gui/panels/info.h"

#include <format>

#include "core/application.h"
#include "gui/layout_manager.h"

namespace gui {

void InfoPanel::render_panel() {
    frame_times_[offset_] = app_.current_update_info().delta_ms;
    offset_               = (offset_ + 1) % frame_times_.size();

    ImGui::SeparatorText("Performance Stats");

    // TODO: support disabling
    static_assert(PERF_STATS_HISTORY_SIZE > 0);

    float avg_ms = frame_times_[0];
    float min_ms = avg_ms;
    float max_ms = avg_ms;

    for (size_t i = 1; i < frame_times_.size(); i++) {
        float ms = frame_times_[i];
        avg_ms += ms;
        min_ms = std::min(min_ms, ms);
        max_ms = std::max(max_ms, ms);
    }
    avg_ms /= static_cast<float>(frame_times_.size());
    float jitter = max_ms - min_ms;

    float avg_fps = 1000.f / avg_ms;
    float min_fps = 1000.f / max_ms;
    float max_fps = 1000.f / min_ms;

    ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "Average: %.2f ms (%.1f FPS)", avg_ms,
                       avg_fps);
    ImGui::Text("Min FPS: %.1f | Max FPS: %.1f", min_fps, max_fps);
    ImGui::Text("Jitter: %.2f ms", jitter);

    ImGui::Separator();

    // 3. The Performance Graph
    // PlotLines args: Label, Values, Count, Offset, Overlay Text, Scale Min, Scale Max, Graph
    // Size
    std::string overlay = std::format(
        "{:.1f} ms", frame_times_[(offset_ - 1 + frame_times_.size()) % frame_times_.size()]);

    ImGui::PlotLines("##FrameGraph", frame_times_.data(), static_cast<int>(frame_times_.size()),
                     static_cast<int>(offset_), overlay.c_str(), 0.0f, 33.3f,
                     ImVec2(ImGui::GetContentRegionAvail().x, 80.0f));
}

} // namespace gui