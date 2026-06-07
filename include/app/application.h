#pragma once

#include <memory>
#include <vector>

#include "app/common.h"
#include "app/input_manager.h"
#include "app/window.h"
#include "gui/gui_manager.h"
#include "imgui_wrapper.h"
#include "render/common.h"
#include "render/render_pipeline.h"
#include "scene.h"
#include "utils/num_cast.h"
#include "vendor.h"

#include "gui/panels/general_config.h"

class Application {
    friend class gui::GuiManager;
    friend class gui::GeneralConfigPanel;

    Window& window_;

    std::vector<std::unique_ptr<IScene>> scenes_{};
    int active_scene_idx_ = -1;

    std::vector<std::unique_ptr<RenderPipeline>> pipelines_{};
    int active_pipeline_idx_ = -1;

    gui::GuiManager gui_manager_;

    InputManager<false> input_manager_;

    UpdateInfo current_update_info_{0.f, 0.f};

    bool terminate_next_frame_ = false;

    // TODO: CPU/GPU dual-storage wrapper, asset loader/manager?
    Mesh suzanne_gpu;
    MeshCPU<> suzanne_cpu;

public:
    explicit Application(Window& window, bool do_init = true)
        : window_{window}, gui_manager_{*this, false}, input_manager_{window_} {
        if (do_init)
            init();
    }

    ~Application() {}

    Application(const Application&)            = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&)                 = delete;
    Application& operator=(Application&&)      = delete;

    void init();

    void run() {
        while (!window_.should_close() && !terminate_next_frame_) {
            main_loop();
        }
    }

    void main_loop();
    void update(const UpdateInfo& update_info);
    void render();

    void switch_scene(int idx);
    void switch_pipeline(int idx);

    std::span<const std::unique_ptr<IScene>> scenes() const { return scenes_; }
    int active_scene_idx() const { return active_scene_idx_; }

    std::span<const std::unique_ptr<RenderPipeline>> pipelines() const { return pipelines_; }
    int active_pipeline_idx() const { return active_pipeline_idx_; }

    const auto& input_manager() const { return input_manager_; }

    const UpdateInfo& current_update_info() const { return current_update_info_; }

    void signal_quit() { terminate_next_frame_ = true; }

    void handle_resize(int width, int height);

private:
    IScene& active_scene() {
        assert(scenes_[num_cast<size_t>(active_scene_idx_)] != nullptr);
        return *scenes_[num_cast<size_t>(active_scene_idx_)];
    }
    const IScene& active_scene() const { return active_scene(); }

    RenderPipeline& active_pipeline() {
        assert(pipelines_[num_cast<size_t>(active_pipeline_idx_)] != nullptr);
        return *pipelines_[num_cast<size_t>(active_pipeline_idx_)];
    }
    const RenderPipeline& active_pipeline() const { return active_pipeline(); }
};
