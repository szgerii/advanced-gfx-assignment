#pragma once

#include <cstdlib>
#include <filesystem>
#include <string_view>

namespace meta {

inline constexpr std::string_view APP_ID   = "P6ODRH_HGrafBead";
inline constexpr std::string_view APP_NAME = "Haladó Grafika Beadandó";

std::filesystem::path app_data_path();

inline std::filesystem::path layouts_dir_path() {
    static const auto path = app_data_path() / "layouts";
    return path;
}

void ensure_data_paths_exist();

} // namespace meta
