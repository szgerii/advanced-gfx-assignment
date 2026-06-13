#pragma once

#include <cstdlib>
#include <filesystem>
#include <string_view>

#ifndef SRC_ROOT
    #error SRC_ROOT macro must be defined during compilation
#endif

namespace meta {

inline constexpr std::string_view APP_ID   = "P6ODRH_HGrafBead";
inline constexpr std::string_view APP_NAME = "Haladó Grafika Beadandó";

std::filesystem::path app_data_path();

inline std::filesystem::path src_root_path() {
    static const std::filesystem::path path{SRC_ROOT};
    return path;
}

inline std::filesystem::path layouts_dir_path() {
    static const auto path = app_data_path() / "layouts";
    return path;
}

inline std::filesystem::path assets_dir_path() {
    static const auto path = src_root_path() / "assets";
    return path;
}

inline std::filesystem::path get_font_path(std::string_view font_name,
                                           std::string_view font_ext = ".ttf") {
    return (assets_dir_path() / "fonts" / font_name).replace_extension(font_ext);
}

inline std::filesystem::path get_model_path(std::string_view model_name,
                                            std::string_view model_ext = ".obj") {
    return (assets_dir_path() / "models" / model_name).replace_extension(model_ext);
}

inline std::filesystem::path get_texture_path(std::string_view tex_name,
                                              std::string_view tex_ext = ".png") {
    return (assets_dir_path() / "textures" / tex_name).replace_extension(tex_ext);
}

inline std::filesystem::path get_shader_path(std::string_view shader_name_with_ext) {
    return assets_dir_path() / "shaders" / shader_name_with_ext;
}

void ensure_app_paths_exist();

} // namespace meta
