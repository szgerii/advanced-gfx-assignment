#include "render/render_pass.h"

#include "render/ogl.h"

#include <algorithm>

std::vector<DrawCommand> IRenderPass::prepare_cmds(const RenderQueue& queue) const {
    VALIDATE(*this);

    auto cmds = queue.get_commands(pass_layer_);
    if (cmds.empty())
        return {};

    // TODO: avoid copy here somehow (queue could sort centrally maybe?)
    std::vector<DrawCommand> sorted{cmds.begin(), cmds.end()};
    std::sort(sorted.begin(), sorted.end(), [](const DrawCommand& a, const DrawCommand& b) {
        // TODO: some better sorting (e.g. weigh shader program and binding differences)
        return &a.interface < &b.interface;
    });

    return sorted;
}
