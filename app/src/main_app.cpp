#include "main_app.h"

#include "render/ogl.h"
#include "render/passes/skybox_pass.h"
#include "scenes/test_scene.h"
#include "utils/image_rgba.h"
#include "utils/obj_parser.h"

// TODO: split into multiple init fns
void MainApp::init(bool do_gl_debug_init) {
    IApplication::init(do_gl_debug_init);

    // SUZANNE
    auto suzanne_cpu = ObjParser::parse("assets/models/Suzanne.obj");

    suzanne_gpu_ = Mesh{.vbo          = std::make_shared<VertexBuffer>(suzanne_cpu.vertices),
                        .ebo          = std::make_shared<IndexBuffer>(suzanne_cpu.indices),
                        .vao          = VertexArray{},
                        .vertex_count = num_cast<uint32_t>(suzanne_cpu.vertices.size()),
                        .index_count  = num_cast<uint32_t>(suzanne_cpu.indices.size())};

    suzanne_gpu_.vao.attach(*suzanne_gpu_.vbo, 0, 0, sizeof(Vertex));
    suzanne_gpu_.vao.attach(*suzanne_gpu_.ebo);
    suzanne_gpu_.vao.config_attribute(0, 3, GL_FLOAT, offsetof(Vertex, pos));
    suzanne_gpu_.vao.config_attribute(1, 3, GL_FLOAT, offsetof(Vertex, normal));
    suzanne_gpu_.vao.config_attribute(2, 2, GL_FLOAT, offsetof(Vertex, texcoord));

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

    skybox_cube_gpu_ = Mesh{.vbo = std::make_shared<VertexBuffer>(skybox_cube_cpu.vertices),
                            .ebo = std::make_shared<IndexBuffer>(skybox_cube_cpu.indices),
                            .vao = VertexArray{},
                            .vertex_count = num_cast<uint32_t>(skybox_cube_cpu.vertices.size()),
                            .index_count  = num_cast<uint32_t>(skybox_cube_cpu.indices.size())};

    skybox_cube_gpu_.vao.attach(*skybox_cube_gpu_.vbo, 0, 0, sizeof(glm::vec3));
    skybox_cube_gpu_.vao.attach(*skybox_cube_gpu_.ebo);
    skybox_cube_gpu_.vao.config_attribute(0, 3, GL_FLOAT, offsetof(glm::vec3, x));

    // SKYBOX TEXTURE
    static constexpr std::array skybox_paths = {
        "assets/textures/skybox/xpos.png", "assets/textures/skybox/xneg.png",
        "assets/textures/skybox/ypos.png", "assets/textures/skybox/yneg.png",
        "assets/textures/skybox/zpos.png", "assets/textures/skybox/zneg.png"};

    ImageRGBA skybox_faces[6];
    for (size_t i = 0; i < 6; i++)
        skybox_faces[i] = ImageRGBA::from_file(skybox_paths[i], false);

    skybox_tex_ = std::make_shared<TextureCube>(num_cast<GLint>(skybox_faces[0].width()));
    for (size_t i = 0; i < 6; i++) {
        auto face_bytes =
            std::span(reinterpret_cast<const uint8_t*>(skybox_faces[i].texels().data()),
                      skybox_faces[i].texels().size() * 4);

        skybox_tex_->upload_face(static_cast<CubeFace>(i), face_bytes);
    }

    // TODO: remove absolute paths
    auto mesh_vert = std::make_shared<Shader>(
        GL_VERTEX_SHADER,
        std::filesystem::path{
            "C:\\Users\\szucs\\projects\\hgraf-bead\\shaders\\pos_norm_tex.vert"});
    auto mesh_frag = std::make_shared<Shader>(
        GL_FRAGMENT_SHADER,
        std::filesystem::path{
            "C:\\Users\\szucs\\projects\\hgraf-bead\\shaders\\simple_lighting.frag"});
    auto mesh_prog = std::make_shared<ShaderProgram>(mesh_vert, mesh_frag);
    mesh_prog->build();

    mesh_iface_ = ProgramInterface(mesh_prog);

    auto skybox_vert = std::make_shared<Shader>(
        GL_VERTEX_SHADER,
        std::filesystem::path{"C:\\Users\\szucs\\projects\\hgraf-bead\\shaders\\skybox.vert"});
    auto skybox_frag = std::make_shared<Shader>(
        GL_FRAGMENT_SHADER,
        std::filesystem::path{"C:\\Users\\szucs\\projects\\hgraf-bead\\shaders\\skybox.frag"});
    auto skybox_prog = std::make_shared<ShaderProgram>(skybox_vert, skybox_frag);
    skybox_prog->build();

    skybox_iface_ = ProgramInterface(skybox_prog);
    skybox_iface_.attach(0, skybox_tex_.get());

    // SCENES
    scenes_.emplace_back(
        std::make_unique<TestScene>(suzanne_gpu_, skybox_cube_gpu_, mesh_iface_, skybox_iface_));

    // PIPELINES
    pipelines_.emplace_back(std::make_unique<RenderPipeline>("Simple Mesh", 1));
    pipelines_.back()->add_pass(std::make_unique<OpaqueRenderPass>());

    pipelines_.emplace_back(std::make_unique<RenderPipeline>("Simple Mesh (Skybox)", 2));
    pipelines_.back()->add_pass(std::make_unique<SkyboxRenderPass>());
    pipelines_.back()->add_pass(std::make_unique<OpaqueRenderPass>());
}
