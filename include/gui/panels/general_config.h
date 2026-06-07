#pragma once

#include "gui/panel.h"

class Application;

namespace gui {

class GeneralConfigPanel final : public IPanel {
    Application& app_;

public:
    explicit GeneralConfigPanel(Application& app)
        : IPanel{"General"}, app_{app} {}

protected:
    void render_panel() override;
};

} // namespace gui
