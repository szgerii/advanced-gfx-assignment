#include "render/passes/skybox_pass.h"

#include "render/ogl.h"

void SkyboxRenderPass::run(const RenderQueue& queue) {
    VALIDATE(*this);

    auto cmds = prepare_cmds(queue);
    if (cmds.empty())
        return;

    glActiveTexture(GL_TEXTURE0);

    ProgramInterface* current_iface = nullptr;
    for (const auto& cmd : cmds) {
        if (cmd.interface != current_iface) {
            if (current_iface != nullptr)
                current_iface->unbind();

            cmd.interface->bind();
            current_iface = cmd.interface;
        }

        cmd.interface->program().upload_uniform("skybox_tex", 0);
        cmd.interface->program().upload_uniform("view_proj", cmd.view_proj);
        cmd.interface->program().upload_uniform("world", cmd.world);

        GLint prev_depth_fn;
        glGetIntegerv(GL_DEPTH_FUNC, &prev_depth_fn);

        glDepthFunc(GL_LEQUAL);

        cmd.mesh->vao.bind();
        OGL::draw_mesh(*cmd.mesh);

        glDepthFunc(num_cast<GLenum>(prev_depth_fn));
    }
}
