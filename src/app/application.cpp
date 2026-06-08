#include "app/application.h"

#include "render/ogl.h"
#include "scenes/test_scene.h"
#include "utils/obj_parser.h"

#include "gui/panels/general_config.h"
#include "gui/panels/imgui_config.h"
#include "gui/panels/info.h"

// TODO: split into multiple init fns
void Application::init() {
    // initialize callbacks before GuiManager initializes imgui
    // this way imgui automatically handles callback "integration" properly
    static auto framebuffer_size_cb =
        FramebufferSizeCallback::from<&Application::handle_resize>(this);
    window_.set_framebuffer_size_callback(framebuffer_size_cb);

    static auto cursor_pos_cb =
        CursorPosCallback::from<&decltype(input_manager_)::record_cursor_pos>(&input_manager_);
    window_.set_cursor_pos_callback(cursor_pos_cb);

    static auto mouse_btn_cb =
        MouseButtonCallback::from<&decltype(input_manager_)::record_mouse_btn>(&input_manager_);
    window_.set_mouse_button_callback(mouse_btn_cb);

    static auto scroll_cb =
        ScrollCallback::from<&decltype(input_manager_)::record_scroll>(&input_manager_);
    window_.set_scroll_callback(scroll_cb);

    static auto key_cb = KeyCallback::from<&decltype(input_manager_)::record_key>(&input_manager_);
    window_.set_key_callback(key_cb);

    gui_manager_.init();
    input_manager_.imgui_initialized = true;

    gui_manager_.attach_panel(std::make_unique<gui::ImGuiConfigPanel>(*this));
    gui_manager_.attach_panel(std::make_unique<gui::GeneralConfigPanel>(*this));
    gui_manager_.attach_panel(std::make_unique<gui::InfoPanel>(*this));

    window_.update_viewport();

    // OGL::set_clear_color(0.35f, 0.25f, 0.875f);
    OGL::set_clear_color(0.1f, 0.1f, 0.1f);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);

    // SUZANNE
    suzanne_cpu = ObjParser::parse("assets/models/Suzanne.obj");

    suzanne_gpu = Mesh{.vbo          = std::make_shared<VertexBuffer>(suzanne_cpu.vertices),
                       .ebo          = std::make_shared<IndexBuffer>(suzanne_cpu.indices),
                       .vao          = VertexArray{},
                       .vertex_count = num_cast<uint32_t>(suzanne_cpu.vertices.size()),
                       .idx_count    = num_cast<uint32_t>(suzanne_cpu.indices.size())};

    suzanne_gpu.vao.attach(*suzanne_gpu.vbo, 0, 0, sizeof(Vertex));
    suzanne_gpu.vao.attach(*suzanne_gpu.ebo);
    suzanne_gpu.vao.config_attribute(0, 3, GL_FLOAT, offsetof(Vertex, pos));
    suzanne_gpu.vao.config_attribute(1, 3, GL_FLOAT, offsetof(Vertex, normal));
    suzanne_gpu.vao.config_attribute(2, 2, GL_FLOAT, offsetof(Vertex, texcoord));

    // PROGRAM INTERFACE (will be moved out from here later [TODO])
    // TODO: remove absolute paths
    auto vert_stage = std::make_shared<Shader>(
        GL_VERTEX_SHADER,
        std::filesystem::path{
            "C:\\Users\\szucs\\projects\\hgraf-bead\\shaders\\pos_norm_tex.vert"});
    auto frag_stage = std::make_shared<Shader>(
        GL_FRAGMENT_SHADER,
        std::filesystem::path{
            "C:\\Users\\szucs\\projects\\hgraf-bead\\shaders\\simple_lighting.frag"});
    auto shader_prog = std::make_shared<ShaderProgram>(vert_stage, frag_stage);
    shader_prog->build();

    auto prog_iface = std::make_shared<ProgramInterface>(shader_prog);
    // prog_iface->attach(0, ubo);

    // SCENES
    scenes_.emplace_back(std::make_unique<TestScene>(suzanne_gpu, prog_iface));

    // PIPELINES
    pipelines_.emplace_back(std::make_unique<RenderPipeline>("Simple Mesh Shading Pipeline", 1));
    pipelines_.back()->add_pass(std::make_unique<OpaqueRenderPass>());

    // update scene cameras and viewport
    handle_resize(window_.properties.width, window_.properties.height);
}

void Application::main_loop() {
    float last_time    = current_update_info_.total_ms;
    float current_time = static_cast<float>(glfwGetTime());

    current_update_info_.total_ms = current_time * 1000.f;
    current_update_info_.delta_ms = current_time * 1000.f - last_time;

    update(current_update_info_);

    render();

    gui_manager_.render_gui();

    window_.swap_buffers();

    input_manager_.new_poll();
    glfwPollEvents();
}

void Application::update(const UpdateInfo& update_info) {
    if (active_scene_idx_ >= 0) {
        auto maybe_cam_controller = active_scene().cam_controller();

        if (maybe_cam_controller.has_value()) {
            auto& cam_controller = maybe_cam_controller->get();

            double mouse_dx = input_manager_.mouse_pos_dx();
            double mouse_dy = input_manager_.mouse_pos_dy();
            if (mouse_dx != 0.0 || mouse_dy != 0.0) {
                cam_controller.process_mouse_move(
                    static_cast<float>(mouse_dx), static_cast<float>(mouse_dy),
                    input_manager_.left_mouse_btn_down(), input_manager_.right_mouse_btn_down());
            }

            double scroll_dx = input_manager_.scroll_dx();
            double scroll_dy = input_manager_.scroll_dy();
            if (scroll_dx != 0.0 || scroll_dy != 0.0) {
                cam_controller.process_mouse_scroll(static_cast<float>(scroll_dx),
                                                    static_cast<float>(scroll_dy));
            }

            for (const auto& key_ev : input_manager_.key_events()) {
                if (key_ev.action == GLFW_PRESS)
                    cam_controller.process_keyboard_down(
                        key_ev.key, input_manager_.key_just_pressed(key_ev.key));
                else if (key_ev.action == GLFW_RELEASE)
                    cam_controller.process_keyboard_up(key_ev.key);
            }
        }
    }

    if (active_scene_idx_ >= 0) {
        auto& scene = active_scene();
        scene.update(update_info);
        scene.update_camera_controller(update_info);
    }

    gui_manager_.update(update_info);
}

void Application::render() {
    OGL::clear(true, true, false);

    if (active_pipeline_idx_ >= 0 && active_scene_idx_ >= 0)
        active_pipeline().run(active_scene());
}

void Application::switch_scene(int idx) {
    if (idx < 0) {
        active_scene_idx_ = -1;
        return;
    }

    if (num_cast<size_t>(idx) >= scenes_.size()) {
        Logger::warn("Application",
                     std::format("trying to set active scene index to {}, but only {} scenes "
                                 "are available. aborting scene switch.",
                                 idx, scenes_.size()));
        return;
    }

    active_scene_idx_ = idx;
}

void Application::switch_pipeline(int idx) {
    if (idx < 0) {
        active_pipeline_idx_ = -1;
        return;
    }

    if (num_cast<size_t>(idx) >= pipelines_.size()) {
        Logger::warn(
            "Application",
            std::format("trying to set active render pipeline index to {}, but only {} pipelines "
                        "are available. aborting render pipeline switch.",
                        idx, pipelines_.size()));
        return;
    }

    active_pipeline_idx_ = idx;
}

void Application::handle_resize(int width, int height) {
    // iconification guard
    if (width <= 0 || height <= 0)
        return;

    OGL::viewport(0, 0, width, height);

    for (auto& scene : scenes_) {
        if (scene != nullptr)
            scene->camera.set_aspect(static_cast<float>(width) / static_cast<float>(height));
    }
}
