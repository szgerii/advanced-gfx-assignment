#pragma once

#include <array>
#include <span>
#include <vector>

#include "core/logger.h"
#include "render/common.h"

class RenderQueue {
    static constexpr size_t DEFAULT_BUFFER_CAPACITY = 32;
    static constexpr size_t RENDER_LAYER_COUNT      = static_cast<size_t>(RenderLayer::COUNT);

    std::array<std::vector<DrawCommand>, RENDER_LAYER_COUNT> cmd_buffers_{};

public:
    explicit RenderQueue() { default_init_buffers(); }
    explicit RenderQueue(std::initializer_list<size_t> buffer_capacities);

    void enqueue(const DrawCommand& cmd) {
        VALIDATE(cmd);
        cmd_buffers_[static_cast<size_t>(cmd.layer)].push_back(cmd);
    }

    std::span<const DrawCommand> get_commands(RenderLayer layer) const {
        VALIDATE(*this);
        VALIDATE_ENUM(RenderLayer, layer);
        return cmd_buffers_[static_cast<size_t>(layer)];
    }

    // preserves capacity
    void clear_buffers() {
        for (auto& buffer : cmd_buffers_)
            buffer.clear();
    }

    DEFINE_VALIDATOR({
        assert(cmd_buffers_.size() == RENDER_LAYER_COUNT);
        for (auto& buffer : cmd_buffers_) {
            VALIDATE_ALL(buffer);
        }
    })

private:
    void default_init_buffers() {
        for (auto& buffer : cmd_buffers_)
            buffer.reserve(DEFAULT_BUFFER_CAPACITY);
    }
};
