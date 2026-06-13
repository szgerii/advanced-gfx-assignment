#include "core/application.h"

#include "gl/debug.h"
#include "gui/panels/general_config.h"
#include "gui/panels/imgui_config.h"
#include "gui/panels/info.h"

void IApplication::init(bool do_init_gl_debug) {
    if (do_init_gl_debug)
        init_gl_debug();

    // initialize callbacks before GuiManager initializes imgui
    // this way imgui automatically handles callback "integration" properly
    static auto framebuffer_size_cb =
        FramebufferSizeCallback::from<&IApplication::handle_resize>(this);
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
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // update scene cameras and viewport
    handle_resize(window_.properties.width, window_.properties.height);
}

void IApplication::init_gl_debug() {
    GLint ctx_flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &ctx_flags);
    if (ctx_flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        static OpenGLDebugCallbackConfig debug_cb_config{.show_notifications = false};

        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // for breakpoints
        glDebugMessageCallback(ogl_debug_msg_callback, &debug_cb_config);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
}

void IApplication::run() {
    while (!window_.should_close() && !terminate_next_frame_) {
        main_loop();
    }
}

void IApplication::main_loop() {
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

void IApplication::update(const UpdateInfo& update_info) {
    if (active_scene_idx_ >= 0) {
        auto& scene = active_scene();

        if (auto maybe_cam_controller = scene.cam_controller(); maybe_cam_controller.has_value()) {
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

        scene.update(update_info);
        scene.update_camera_controller(update_info);
    }

    gui_manager_.update(update_info);
}

void IApplication::render() {
    OGL::clear(true, true, false);

    if (active_pipeline_idx_ >= 0 && active_scene_idx_ >= 0)
        active_pipeline().run(active_scene());
}

void IApplication::switch_scene(int idx) {
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

void IApplication::switch_pipeline(int idx) {
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

void IApplication::handle_resize(int width, int height) {
    // iconification guard
    if (width <= 0 || height <= 0)
        return;

    OGL::viewport(0, 0, width, height);

    for (auto& scene : scenes_) {
        if (scene != nullptr)
            scene->camera.set_aspect(static_cast<float>(width) / static_cast<float>(height));
    }
}
