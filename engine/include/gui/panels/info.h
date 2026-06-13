#pragma once

#include <array>

#include "gui/panel.h"

class IApplication;

namespace gui {

class InfoPanel final : public IPanel {
    // TODO: move these to a separate util class independent of InfoPanel
    static constexpr size_t PERF_STATS_HISTORY_SIZE = 120;

    std::array<float, PERF_STATS_HISTORY_SIZE> frame_times_{};
    size_t offset_ = 0;

    IApplication& app_;

public:
    explicit InfoPanel(IApplication& app)
        : IPanel{"Info"}, app_{app} {
        frame_times_.fill(16.666f);
    }

protected:
    void render_panel() override;
};

} // namespace gui
