#pragma once

#include "gui/panel.h"

class IApplication;

namespace gui {

class GeneralConfigPanel final : public IPanel {
    IApplication& app_;

public:
    explicit GeneralConfigPanel(IApplication& app)
        : IPanel{"General"}, app_{app} {}

protected:
    void render_panel() override;
};

} // namespace gui
