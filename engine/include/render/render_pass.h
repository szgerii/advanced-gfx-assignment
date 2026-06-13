#pragma once

#include <span>
#include <vector>

#include "render/render_queue.h"

class IRenderPass {
    RenderLayer pass_layer_;

public:
    std::string name;

    IRenderPass(std::string name, RenderLayer pass_layer)
        : pass_layer_{pass_layer}, name{std::move(name)} {
        VALIDATE(*this);
    }

    virtual ~IRenderPass() = default;

    IRenderPass(const IRenderPass&)            = delete;
    IRenderPass& operator=(const IRenderPass&) = delete;
    IRenderPass(IRenderPass&&)                 = delete;
    IRenderPass& operator=(IRenderPass&&)      = delete;

    virtual void run(const RenderQueue& queue) = 0;

    DEFINE_VALIDATOR({ VALIDATE_ENUM(RenderLayer, pass_layer_); })

protected:
    std::vector<DrawCommand> prepare_cmds(const RenderQueue& queue) const;
};
