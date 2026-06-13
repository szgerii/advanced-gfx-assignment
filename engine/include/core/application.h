#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "core/common.h"
#include "core/input_manager.h"
#include "core/scene.h"
#include "core/window.h"
#include "gui/gui_manager.h"
#include "gui/imgui_wrapper.h"
#include "gui/panels/general_config.h"
#include "render/common.h"
#include "render/render_pipeline.h"
#include "utils/num_cast.h"
#include "vendor.h"

class IApplication {
    friend class gui::GuiManager;
    friend class gui::GeneralConfigPanel;

protected:
#ifndef NDEBUG
    static constexpr bool DEFAULT_GL_DEBUG_VALUE = true;
#else
    static constexpr bool DEFAULT_GL_DEBUG_VALUE = false;
#endif

private:
    // allows optional app-level window ownership, without having to force the Window to the heap
    // window_ should be used everywhere for access (it'll point to window_data_'s underlying Window
    // when window is app-owned)
    std::optional<Window> window_data_;

protected:
    Window& window_;

    std::vector<std::unique_ptr<IScene>> scenes_{};
    int active_scene_idx_ = -1;

    std::vector<std::unique_ptr<RenderPipeline>> pipelines_{};
    int active_pipeline_idx_ = -1;

    gui::GuiManager gui_manager_;

    InputManager<false> input_manager_;

    UpdateInfo current_update_info_{0.f, 0.f};

    bool terminate_next_frame_ = false;

public:
    std::optional<Window> helper_util_thing(bool create_new) {
        if (create_new)
            return Window{};

        return std::nullopt;
    }

    explicit IApplication(std::optional<std::reference_wrapper<Window>> window = std::nullopt)
        : window_data_{std::move(helper_util_thing(!window.has_value()))},
          window_{window_data_.has_value() ? *window_data_ : (*window).get()},
          gui_manager_{*this, false},
          input_manager_{window_} {}

    virtual ~IApplication() {}

    IApplication(const IApplication&)            = delete;
    IApplication& operator=(const IApplication&) = delete;
    IApplication(IApplication&&)                 = delete;
    IApplication& operator=(IApplication&&)      = delete;

    virtual void init(bool do_init_gl_debug = DEFAULT_GL_DEBUG_VALUE);
    virtual void run();

    void switch_scene(int idx);
    void switch_pipeline(int idx);

    std::span<const std::unique_ptr<IScene>> scenes() const { return scenes_; }
    int active_scene_idx() const { return active_scene_idx_; }

    std::span<const std::unique_ptr<RenderPipeline>> pipelines() const { return pipelines_; }
    int active_pipeline_idx() const { return active_pipeline_idx_; }

    const auto& input_manager() const { return input_manager_; }
    const auto& gui_manager() const { return gui_manager_; }
    const auto& window() const { return window_; }

    const UpdateInfo& current_update_info() const { return current_update_info_; }

    void signal_quit() { terminate_next_frame_ = true; }

    virtual void handle_resize(int width, int height);

protected:
    virtual void init_gl_debug();

    virtual void main_loop();

    virtual void update(const UpdateInfo& update_info);
    virtual void render();

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
