#pragma once

#include "render/render_pass.h"

class OpaqueRenderPass : public IRenderPass {
public:
    explicit OpaqueRenderPass()
        : IRenderPass{"Opaque Render Pass", RenderLayer::Opaque} {}

    virtual void run(const RenderQueue& queue) override;
};
