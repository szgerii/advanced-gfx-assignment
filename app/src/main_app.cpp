#include "main_app.h"

#include "core/meta.h"
#include "render/ogl.h"
#include "render/passes/skybox_pass.h"
#include "scenes/test_scene.h"
#include "utils/image_rgba.h"
#include "utils/obj_parser.h"

// TODO: split into multiple init fns
void MainApp::init(bool do_gl_debug_init) {
    IApplication::init(do_gl_debug_init);

    // SUZANNE
    auto suzanne_cpu = ObjParser::parse(meta::get_model_path("Suzanne"));

    MeshGPU suzanne_gpu{.vbo            = std::make_shared<VertexBuffer>(suzanne_cpu.vertices),
                        .ibo            = std::make_shared<IndexBuffer>(suzanne_cpu.indices),
                        .meshlet_buffer = nullptr,
                        .vao            = std::make_shared<VertexArray>(),
                        .vertex_count   = num_cast<uint32_t>(suzanne_cpu.vertices.size()),
                        .index_count    = num_cast<uint32_t>(suzanne_cpu.indices.size()),
                        .meshlet_count  = 0,
                        .mesh_data_type = MeshDataType::Indexed};

    suzanne_gpu.vao->attach(*suzanne_gpu.vbo, 0, 0, sizeof(Vertex));
    suzanne_gpu.vao->attach(*suzanne_gpu.ibo);
    suzanne_gpu.vao->config_attribute(0, 3, GL_FLOAT, offsetof(Vertex, pos));
    suzanne_gpu.vao->config_attribute(1, 3, GL_FLOAT, offsetof(Vertex, normal));
    suzanne_gpu.vao->config_attribute(2, 2, GL_FLOAT, offsetof(Vertex, tex));

    suzanne_mesh_ = std::make_unique<BasicMesh>(suzanne_cpu, suzanne_gpu, false);

    // SKYBOX CUBE
    // clang-format off
    MeshCPU<glm::vec3> skybox_cube_cpu{
        .vertices = {
            // back
            glm::vec3(-1, -1, -1),
            glm::vec3( 1, -1, -1),
            glm::vec3( 1,  1, -1),
            glm::vec3(-1,  1, -1),
            // front
            glm::vec3(-1, -1,  1),
            glm::vec3( 1, -1,  1),
            glm::vec3( 1,  1,  1),
            glm::vec3(-1,  1,  1)
        },
        .indices = {
            // back
            0, 1, 2,
            2, 3, 0,
            // front
            4, 6, 5,
            6, 4, 7,
            // left
            0, 3, 4,
            4, 3, 7,
            // right
            1, 5, 2,
            5, 6, 2,
            // bottom
            1, 0, 4,
            1, 4, 5,
            // top
            3, 2, 6,
            3, 6, 7,
        }
    };
    // clang-format on

    MeshGPU skybox_cube_gpu{.vbo = std::make_shared<VertexBuffer>(skybox_cube_cpu.vertices),
                            .ibo = std::make_shared<IndexBuffer>(skybox_cube_cpu.indices),
                            .meshlet_buffer = nullptr,
                            .vao            = std::make_shared<VertexArray>(),
                            .vertex_count   = num_cast<uint32_t>(skybox_cube_cpu.vertices.size()),
                            .index_count    = num_cast<uint32_t>(skybox_cube_cpu.indices.size()),
                            .meshlet_count  = 0,
                            .mesh_data_type = MeshDataType::Indexed};

    skybox_cube_gpu.vao->attach(*skybox_cube_gpu.vbo, 0, 0, sizeof(glm::vec3));
    skybox_cube_gpu.vao->attach(*skybox_cube_gpu.ibo);
    skybox_cube_gpu.vao->config_attribute(0, 3, GL_FLOAT, offsetof(glm::vec3, x));

    skybox_cube_mesh_ = std::make_unique<MinimalMesh>(skybox_cube_cpu, skybox_cube_gpu, false);

    // SKYBOX TEXTURE
    static constexpr std::array skybox_paths = {"skybox/xpos", "skybox/xneg", "skybox/ypos",
                                                "skybox/yneg", "skybox/zpos", "skybox/zneg"};

    ImageRGBA skybox_faces[6];
    for (size_t i = 0; i < 6; i++)
        skybox_faces[i] = ImageRGBA::from_file(meta::get_texture_path(skybox_paths[i]), false);

    skybox_tex_ = std::make_shared<TextureCube>(num_cast<GLint>(skybox_faces[0].width()));
    for (size_t i = 0; i < 6; i++) {
        auto face_bytes =
            std::span(reinterpret_cast<const uint8_t*>(skybox_faces[i].texels().data()),
                      skybox_faces[i].texels().size() * 4);

        skybox_tex_->upload_face(static_cast<CubeFace>(i), face_bytes);
    }

    auto mesh_vert =
        std::make_shared<Shader>(GL_VERTEX_SHADER, meta::get_shader_path("pos_norm_tex.vert"));
    auto mesh_frag =
        std::make_shared<Shader>(GL_FRAGMENT_SHADER, meta::get_shader_path("simple_lighting.frag"));
    auto mesh_prog = std::make_shared<ShaderProgram>(mesh_vert, mesh_frag);
    mesh_prog->build();

    mesh_iface_ = ProgramInterface(mesh_prog);

    auto skybox_vert =
        std::make_shared<Shader>(GL_VERTEX_SHADER, meta::get_shader_path("skybox.vert"));
    auto skybox_frag =
        std::make_shared<Shader>(GL_FRAGMENT_SHADER, meta::get_shader_path("skybox.frag"));
    auto skybox_prog = std::make_shared<ShaderProgram>(skybox_vert, skybox_frag);
    skybox_prog->build();

    skybox_iface_ = ProgramInterface(skybox_prog);
    skybox_iface_.attach(0, skybox_tex_.get());

    // SCENES
    scenes_.emplace_back(std::make_unique<TestScene>(*suzanne_mesh_, *skybox_cube_mesh_,
                                                     mesh_iface_, skybox_iface_));

    // PIPELINES
    pipelines_.emplace_back(std::make_unique<RenderPipeline>("Simple Mesh", 1));
    pipelines_.back()->add_pass(std::make_unique<OpaqueRenderPass>());

    pipelines_.emplace_back(std::make_unique<RenderPipeline>("Simple Mesh (Skybox)", 2));
    pipelines_.back()->add_pass(std::make_unique<SkyboxRenderPass>());
    pipelines_.back()->add_pass(std::make_unique<OpaqueRenderPass>());
}
