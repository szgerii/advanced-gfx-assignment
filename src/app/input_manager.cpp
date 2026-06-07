#include "app/input_manager.h"

namespace {

std::string with_int(std::string_view base, int value) {
    return std::format("{} [value: {}]", base, value);
}

std::string mods_str(int mods) {
    std::string result = "";

    // clang-format off
    if (mods & GLFW_MOD_CONTROL) result += "ctrl+";
    if (mods & GLFW_MOD_ALT)     result += "alt+";
    if (mods & GLFW_MOD_SHIFT)   result += "shift+";
    if (mods & GLFW_MOD_SUPER)   result += "super+";
    // clang-format on

    if (!result.empty())
        result.pop_back();
    else
        result = "none";

    return with_int(result, mods);
}

std::string action_str(int action) {
    if (action == GLFW_PRESS)
        return with_int("press", action);
    if (action == GLFW_RELEASE)
        return with_int("release", action);
    if (action == GLFW_REPEAT)
        return with_int("repeat", action);

    return with_int("unknown", action);
}

std::string mouse_btn_str(int btn) {
    if (btn == GLFW_MOUSE_BUTTON_LEFT)
        return with_int("left", btn);
    if (btn == GLFW_MOUSE_BUTTON_RIGHT)
        return with_int("right", btn);
    if (btn == GLFW_MOUSE_BUTTON_MIDDLE)
        return with_int("middle", btn);

    size_t idx = 0;
    for (int it = GLFW_MOUSE_BUTTON_1; it <= GLFW_MOUSE_BUTTON_LAST; it++, idx++) {
        if (it == btn)
            return with_int(std::format("#{}", idx), btn);
    }

    return with_int("unknown", btn);
}

} // namespace

namespace detail {

std::string input_event_str(InputEvent ev) {
    return std::visit(Overloaded{[](const KeyEvent& ev) {
                                     const char* key_name = glfwGetKeyName(ev.key, ev.scan_code);
                                     return std::format("Recorded key input:\n"
                                                        "Key: {} [key: {}, scan code: {}]\n"
                                                        "Action: {}\n"
                                                        "Active mods: {}",
                                                        key_name ? key_name : "unknown", ev.key,
                                                        ev.scan_code, action_str(ev.action),
                                                        mods_str(ev.mods));
                                 },
                                 [](const MouseButtonEvent& ev) {
                                     return std::format("Recorded mouse button event:\n"
                                                        "Button: {}\n"
                                                        "Action: {}\n"
                                                        "Active mods: {}",
                                                        mouse_btn_str(ev.btn),
                                                        action_str(ev.action), mods_str(ev.mods));
                                 },
                                 [](const CursorPosEvent& ev) {
                                     return std::format("Recorded cursor pos event:\n"
                                                        "x: {} | y: {}",
                                                        ev.x, ev.y);
                                 },
                                 [](const ScrollEvent& ev) {
                                     return std::format("Recorded scroll event:\n"
                                                        "dx: {} | dx: {}",
                                                        ev.dx, ev.dy);
                                 }},
                      ev);
}

} // namespace detail
