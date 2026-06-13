#pragma once

#include <memory>

#include "core/application.h"
#include "core/common.h"
#include "core/mesh.h"
#include "gl/textures.h"
#include "render/common.h"

class MainApp final : public IApplication {
    // TODO: CPU/GPU dual-storage wrapper, asset loader/manager?
    std::unique_ptr<BasicMesh> suzanne_mesh_{};
    std::unique_ptr<MinimalMesh> skybox_cube_mesh_{};

    std::shared_ptr<TextureCube> skybox_tex_{};

    ProgramInterface mesh_iface_;
    ProgramInterface skybox_iface_;

public:
    using IApplication::IApplication;

    void init(bool do_init_gl_debug = DEFAULT_GL_DEBUG_VALUE) override;
};
