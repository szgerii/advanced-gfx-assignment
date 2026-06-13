#pragma once

#include <array>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "core/logger.h"
#include "core/window.h"
#include "gui/imgui_wrapper.h"
#include "utils/common.h"
#include "utils/num_cast.h"
#include "vendor.h"

// TODO: independent key/btn/etc. types instead of GLFW enums

// these event types are (for now) meant to be interpreted 1:1 as their GLFW counterparts

struct CursorPosEvent {
    double x;
    double y;
};

struct ScrollEvent {
    double dx;
    double dy;
};

struct MouseButtonEvent {
    int btn;
    int action;
    int mods;
};

struct KeyEvent {
    int key;
    int scan_code;
    int action;
    int mods;
};

using InputEvent = std::variant<KeyEvent, MouseButtonEvent, CursorPosEvent, ScrollEvent>;

namespace detail {

std::string input_event_str(InputEvent ev);

} // namespace detail

// TODO: state debugging (logging) would be nice

// designed to collect and aggregate data from multiple callbacks happening in the same poll
template <bool StoreEventQueue = true, bool DebugEventQueue = false>
class InputManager {
    static constexpr size_t EXPECTED_EVENTS_PER_POLL = 32;

    // debug param forces manager into storage mode
    // this controls the actual storage mechanism
    static constexpr bool StoresEventQueue = StoreEventQueue || DebugEventQueue;

    Window& window_;

    // events will be recorded in order into this if StoreEventQueue is enabled
    std::vector<InputEvent> event_queue_{};

    // running variables constantly updated per polling:
    // use new_poll() to reset them before polling input events

    std::vector<MouseButtonEvent> mouse_btn_events_{};       // collected
    std::vector<KeyEvent> key_events_{};                     // collected
    double scroll_dx_, scroll_dy_;                           // summed
    double mouse_pos_dx_, mouse_pos_dy_;                     // summed
    double mouse_pos_x_, mouse_pos_y_;                       // latest
    bool left_mouse_btn_down_, right_mouse_btn_down_,        // latest*
        left_mouse_just_pressed_, right_mouse_just_pressed_; // latest/buffered*
    std::array<bool, GLFW_KEY_LAST + 1> key_down_{};         // latest*
    std::array<bool, GLFW_KEY_LAST + 1> key_just_pressed_{}; // latest/buffered*

    // internal state stuff:

    // we save just_pressed state into the above members instead of checking against this at
    // query-time, because this way even if during a single poll the mouse goes through the
    // following state change: released -> pressed -> released, the click action will be captured
    // (it's also why we don't provide just_released, as then the above interaction would yield
    // just_pressed and just_released at the same time. if you need sub-frame precision, you should
    // be using the queues anyways)
    bool prev_left_mouse_down_;
    bool prev_right_mouse_down_;

    // same goes for keyboard states
    std::array<bool, GLFW_KEY_LAST + 1> prev_key_down_{};

    bool in_imgui_mouse_lock = false;
    bool in_imgui_kb_lock    = false;

    double last_mouse_pos_x_ = -1.0;
    double last_mouse_pos_y_ = -1.0;

    // *:
    // note that for API consistency, key_just_pressed(K) implies key_down(K).
    // this means that it's possible that the last entry for K in key_events() is a release, but
    // (key_down(K) == true) holds. this happens only when a key is pressed and released in the
    // same polling cycle.
    // more precisely:
    // whenever key_events() indicates K is released, key_down(K) iff key_just_pressed(K)
    // same goes for mouse button states

    // if higher precision is required, always use one of the queues provided, which are never
    // altered in any way for convenience

public:
    // determines whether ImGui can be queried for capture info
    // it's assumed that polling is frequent enough that the current state does not need to be
    // updated before the next call to new_poll
    // this is so that queries remain deterministic per poll (save for loose record calls)
    // this allows us to gracefully handle every input we recorded pre-imgui-capture, and only
    // release keys and buttons on the initialization of the next poll
    bool imgui_initialized;

    explicit InputManager(Window& window, bool imgui_initialized = false)
        : window_{window}, imgui_initialized{imgui_initialized} {
        if constexpr (StoresEventQueue) {
            event_queue_.reserve(EXPECTED_EVENTS_PER_POLL);
        }

        if constexpr (StoresEventQueue && !StoreEventQueue) {
            Logger::warn("Input",
                         "global event queue storage implicitly enabled for input manager (this is "
                         "most likely a result of event queue debugging being enabled)");
        }

        sync_from_glfw();
    }

    const std::vector<InputEvent>& event_queue() const { return event_queue_; }

    const std::vector<MouseButtonEvent>& mouse_btn_events() const { return mouse_btn_events_; }
    const std::vector<KeyEvent>& key_events() const { return key_events_; }

    double mouse_pos_x() const { return mouse_pos_x_; }
    double mouse_pos_y() const { return mouse_pos_y_; }

    double mouse_pos_dx() const { return mouse_pos_dx_; }
    double mouse_pos_dy() const { return mouse_pos_dy_; }

    double scroll_dx() const { return scroll_dx_; }
    double scroll_dy() const { return scroll_dy_; }

    // *
    bool left_mouse_btn_down() const { return left_mouse_just_pressed_ || left_mouse_btn_down_; }

    // *
    bool right_mouse_btn_down() const { return right_mouse_just_pressed_ || right_mouse_btn_down_; }

    bool left_mouse_just_pressed() const { return left_mouse_just_pressed_; }
    bool right_mouse_just_pressed() const { return right_mouse_just_pressed_; }

    bool key_down(int key) const {
        assert(0 <= key && key <= GLFW_KEY_LAST && "invalid key value");
        return key_just_pressed_[num_cast<size_t>(key)] || key_down_[num_cast<size_t>(key)]; // *
    }

    bool key_just_pressed(int key) const {
        assert(0 <= key && key <= GLFW_KEY_LAST && "invalid key value");
        return key_just_pressed_[num_cast<size_t>(key)];
    }

    // might be different from StoreEventQueue, see StoresEventQueue
    // (but always compile-time known)
    consteval bool stores_event_queue() const { return StoresEventQueue; }

    void new_poll() {
        if constexpr (StoresEventQueue)
            event_queue_.clear();

        mouse_btn_events_.clear();
        key_events_.clear();

        mouse_pos_dx_ = 0.0;
        mouse_pos_dy_ = 0.0;

        scroll_dx_ = 0.0;
        scroll_dy_ = 0.0;

        prev_left_mouse_down_  = left_mouse_btn_down_;
        prev_right_mouse_down_ = right_mouse_btn_down_;

        left_mouse_just_pressed_  = false;
        right_mouse_just_pressed_ = false;

        prev_key_down_ = key_down_;
        std::fill(key_just_pressed_.begin(), key_just_pressed_.end(), false);

        imgui_guard();
    }

    void record_cursor_pos(double x, double y) {
        if (imgui_wants_mouse())
            return;

        push_event(std::in_place_type<CursorPosEvent>, x, y);

        mouse_pos_x_ = x;
        mouse_pos_y_ = y;

        mouse_pos_dx_ += (x - last_mouse_pos_x_);
        mouse_pos_dy_ += (y - last_mouse_pos_y_);
        last_mouse_pos_x_ = x;
        last_mouse_pos_y_ = y;
    }

    void record_scroll(double dx, double dy) {
        if (imgui_wants_mouse())
            return;

        push_event(std::in_place_type<ScrollEvent>, dx, dy);

        scroll_dx_ += dx;
        scroll_dy_ += dy;
    }

    void record_mouse_btn(int btn, int action, int mods) {
        if (imgui_wants_mouse())
            return;

        MouseButtonEvent btn_ev(btn, action, mods);
        push_event(btn_ev);
        mouse_btn_events_.push_back(btn_ev);

        if (btn == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS)
                left_mouse_btn_down_ = true;
            else if (action == GLFW_RELEASE)
                left_mouse_btn_down_ = false;

            if (left_mouse_btn_down_ && !prev_left_mouse_down_)
                left_mouse_just_pressed_ = true;
        } else if (btn == GLFW_MOUSE_BUTTON_RIGHT) {
            if (action == GLFW_PRESS)
                right_mouse_btn_down_ = true;
            else if (action == GLFW_RELEASE)
                right_mouse_btn_down_ = false;

            if (right_mouse_btn_down_ && !prev_right_mouse_down_)
                right_mouse_just_pressed_ = true;
        }
    }

    void record_key(int key, int scan_code, int action, int mods) {
        if (imgui_wants_keyboard())
            return;

        KeyEvent key_ev(key, scan_code, action, mods);
        push_event(key_ev);
        key_events_.push_back(key_ev);

        // skips GLFW_KEY_UNKNOWN (and whatever else that would be an error)
        if (key >= 0 && key <= GLFW_KEY_LAST) {
            size_t key_u = num_cast<size_t>(key);

            if (action == GLFW_PRESS)
                key_down_[key_u] = true;
            else if (action == GLFW_RELEASE)
                key_down_[key_u] = false;

            if (key_down_[key_u] && !prev_key_down_[key_u])
                key_just_pressed_[key_u] = true;
        }
    }

private:
    // args must be forwardable to std::vector<InputEvent>'s emplace_back
    template <typename... Args>
    void push_event(Args&&... args) {
        if constexpr (StoresEventQueue) {
            event_queue_.emplace_back(std::forward<Args>(args)...);

            if constexpr (DebugEventQueue) {
                Logger::debug("Input", detail::input_event_str(event_queue_.back()));
            }
        }
    }

    void sync_from_glfw(bool sync_kb = true, bool sync_mouse = true) {
        if (sync_kb) {
            for (size_t key = GLFW_KEY_SPACE; key < GLFW_KEY_LAST + 1; key++)
                key_down_[key] =
                    (glfwGetKey(window_.get_handle(), static_cast<int>(key)) == GLFW_PRESS);

            // don't indicate just pressed on next frame for keys already held down
            prev_key_down_ = key_down_;
        }

        if (sync_mouse) {
            left_mouse_btn_down_ =
                glfwGetMouseButton(window_.get_handle(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
            right_mouse_btn_down_ =
                glfwGetMouseButton(window_.get_handle(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

            prev_left_mouse_down_  = left_mouse_btn_down_;
            prev_right_mouse_down_ = right_mouse_btn_down_;

            glfwGetCursorPos(window_.get_handle(), &last_mouse_pos_x_, &last_mouse_pos_y_);
        }
    }

    bool imgui_wants_keyboard() const {
        return imgui_initialized && ImGui::GetIO().WantCaptureKeyboard;
    }

    bool imgui_wants_mouse() const { return imgui_initialized && ImGui::GetIO().WantCaptureMouse; }

    void imgui_guard() {
        if (!imgui_initialized)
            return;

        ImGuiIO& io = ImGui::GetIO();

        if (io.WantCaptureMouse && !in_imgui_mouse_lock) {
            // enter mouse lock
            left_mouse_btn_down_ = prev_left_mouse_down_ = false;
            right_mouse_btn_down_ = prev_right_mouse_down_ = false;
        } else if (!io.WantCaptureMouse && in_imgui_mouse_lock) {
            // leave mouse lock
            sync_from_glfw(false, true);
        }

        if (io.WantCaptureKeyboard && !in_imgui_kb_lock) {
            // enter keyboard lock
            std::fill(key_down_.begin(), key_down_.end(), false);
            std::fill(prev_key_down_.begin(), prev_key_down_.end(), false);
        } else if (!io.WantCaptureKeyboard && in_imgui_kb_lock) {
            // leave keyboard lock
            sync_from_glfw(true, false);
        }

        in_imgui_mouse_lock = io.WantCaptureMouse;
        in_imgui_kb_lock    = io.WantCaptureKeyboard;
    }
};
