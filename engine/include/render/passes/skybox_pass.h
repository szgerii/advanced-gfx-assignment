#pragma once

#include "render/render_pass.h"

class SkyboxRenderPass : public IRenderPass {
public:
    explicit SkyboxRenderPass()
        : IRenderPass{"Skybox Render Pass", RenderLayer::Background} {}

    virtual void run(const RenderQueue& queue) override;
};
