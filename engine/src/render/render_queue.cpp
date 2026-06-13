#include "render/render_queue.h"

RenderQueue::RenderQueue(std::initializer_list<size_t> buffer_capacities) {
    if (buffer_capacities.size() != 0 && buffer_capacities.size() != RENDER_LAYER_COUNT) {
        Logger::warn("RenderQueue", std::format("invalid buffer_capacities initializer list: "
                                                "expected 0 or {} arguments, got {} instead. "
                                                "Falling back to default initialization.",
                                                RENDER_LAYER_COUNT, buffer_capacities.size()));
        Logger::debug(
            "RenderQueue",
            std::format(
                "Pass 0 arguments to the RenderQueue constructor to have all buffers "
                "initialized to DEFAULT_BUFFER_CAPACITY ({}), or pass exactly as many "
                "arguments as there are render layers ({}) to have buffers initialized "
                "individually with custom capacities. You can programatically query the render "
                "layer count via RenderLayer::COUNT.",
                DEFAULT_BUFFER_CAPACITY, RENDER_LAYER_COUNT));

        default_init_buffers();
        return;
    }

    assert(cmd_buffers_.size() == RENDER_LAYER_COUNT);
    assert(buffer_capacities.size() == RENDER_LAYER_COUNT);

    auto buffer_it   = cmd_buffers_.begin();
    auto capacity_it = buffer_capacities.begin();
    for (; buffer_it != cmd_buffers_.end(); buffer_it++, capacity_it++)
        buffer_it->reserve(*capacity_it);

    assert(buffer_it == cmd_buffers_.end());
    assert(capacity_it == buffer_capacities.end());
}
