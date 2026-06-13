#include "render/passes/opaque_pass.h"

#include "render/ogl.h"

void OpaqueRenderPass::run(const RenderQueue& queue) {
    VALIDATE(*this);

    auto cmds = prepare_cmds(queue);
    if (cmds.empty())
        return;

    ProgramInterface* current_iface = nullptr;
    for (const auto& cmd : cmds) {
        if (cmd.interface != current_iface) {
            if (current_iface != nullptr)
                current_iface->unbind();

            cmd.interface->bind();
            current_iface = cmd.interface;
        }

        // TODO: UBOs instead of this

        current_iface->program().upload_uniform("world", cmd.world);
        current_iface->program().upload_uniform("world_inv_trans",
                                                glm::transpose(glm::inverse(cmd.world)));

        current_iface->program().upload_uniform("view_proj", cmd.view_proj);

        cmd.mesh->vao->bind();

        OGL::draw_mesh(*cmd.mesh);
    }
}
