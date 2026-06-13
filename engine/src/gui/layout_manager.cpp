#include "gui/layout_manager.h"

#include <algorithm>
#include <cmath>
#include <format>
#include <system_error>

#include "core/logger.h"
#include "core/meta.h"
#include "gui/imgui_wrapper.h"

namespace {

namespace fs = std::filesystem;

// assumes data paths already exist
std::string get_ini_path(std::string_view name, bool must_exist = true) {
    fs::path ini_path{meta::layouts_dir_path() / name};
    if (!ini_path.has_extension() || ini_path.extension() != ".ini")
        ini_path.replace_extension(".ini");

    std::error_code ec;

    if (must_exist && (!fs::exists(ini_path, ec) || ec))
        return "";

    fs::path abs_path = fs::absolute(ini_path, ec);
    return !ec ? abs_path.string() : "";
}

} // namespace

namespace gui {

void LayoutManager::try_load_layout(std::string_view name) {
    meta::ensure_app_paths_exist();

    std::string ini_path = get_ini_path(name);
    if (ini_path.empty())
        return;

    ini_filename_buffer        = std::move(ini_path);
    ImGui::GetIO().IniFilename = ini_filename_buffer.c_str();

    ImGui::LoadIniSettingsFromDisk(ini_filename_buffer.c_str());
}

void LayoutManager::try_save_layout_as(std::string_view name) {
    meta::ensure_app_paths_exist();

    std::string ini_path = get_ini_path(name, false);
    assert(!ini_path.empty());

    ini_filename_buffer        = std::move(ini_path);
    ImGui::GetIO().IniFilename = ini_filename_buffer.c_str();

    ImGui::SaveIniSettingsToDisk(ini_filename_buffer.c_str());
}

bool LayoutManager::layout_exists(std::string_view name) {
    meta::ensure_app_paths_exist();
    return !get_ini_path(name).empty();
}

const std::vector<std::string>&
LayoutManager::get_layout_names(std::optional<float> current_total_ms, bool force_reload) {
    static constexpr float UPDATE_INTERVAL_MS = 5000;

    static std::vector<std::string> result_buffer{};
    static std::optional<float> last_update_total_ms{};

    // shouldn't really happen but let's be safe
    if (last_update_total_ms.has_value() && current_total_ms.has_value() &&
        current_total_ms < last_update_total_ms)
        force_reload = true;

    if (force_reload || !current_total_ms.has_value() || !last_update_total_ms.has_value() ||
        (*current_total_ms - *last_update_total_ms) >= UPDATE_INTERVAL_MS) {
        meta::ensure_app_paths_exist();

        std::error_code ec;
        auto dir_it = fs::directory_iterator(meta::layouts_dir_path(), ec);
        if (!ec) {
            result_buffer.clear();

            for (const auto& entry : dir_it) {
                std::error_code entry_ec;
                bool is_regular_file = entry.is_regular_file(entry_ec);

                if (entry_ec) {
                    Logger::warn("LayoutManager",
                                 std::format("Failed to fetch metadata for file layout '{}', "
                                             "omitting from layout list for current fetch.",
                                             entry.path().stem().string()));
                    continue;
                }

                if (is_regular_file)
                    result_buffer.emplace_back(entry.path().stem().string());
            }

            std::sort(result_buffer.begin(), result_buffer.end());
        } else {
            Logger::warn("LayoutManager",
                         "Failed to fetch file list of layouts directory, keeping "
                         "the previously cached state for one more cycle. Layout loads may fail "
                         "silently if their configs were removed from disk (or are otherwise "
                         "inaccessible).");
        }

        if (current_total_ms.has_value())
            last_update_total_ms = current_total_ms;
    }

    return result_buffer;
}

} // namespace gui
