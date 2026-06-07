#include "meta.h"

#include <cstdlib>
#include <optional>
#include <system_error>

#include "logger.h"

namespace {

namespace fs = std::filesystem;

}

namespace meta {

// source for paths: https://softwareengineering.stackexchange.com/a/3990
std::filesystem::path app_data_path() {
    static const auto path = [] -> std::optional<fs::path> {
        // for lack of a better option
        auto fallback_dir = []() -> std::optional<fs::path> {
            std::error_code ec;
            fs::path cwd = fs::current_path(ec);

            if (ec)
                return std::nullopt;

            return cwd;
        };

#ifdef _WIN32
        const char* appdata = std::getenv("APPDATA");
        if (appdata == nullptr)
            return fallback_dir();

        return fs::path{appdata} / APP_ID;
#elif defined(__APPLE__)
        const char* home = std::getenv("HOME");
        if (home == nullptr)
            return fallback_dir();

        return fs::path{home} / "Library" / "Preferences" / APP_ID;
#elif defined(__linux__)
        const char* xdg = std::getenv("XDG_CONFIG_HOME");
        if (xdg != nullptr)
            return fs::path{xdg} / APP_ID;

        const char* home = std::getenv("HOME");
        if (home == nullptr)
            return fallback_dir();

        return fs::path{home} / ".config" / APP_ID;
#else
        return fallback_dir();
#endif
    }();

    if (!path.has_value()) {
        Logger::critical_error(
            "App",
            "Failed to locate path for app directory (not even current path was "
            "accessible). Check for potential access or ownership issues on your filesystem.");
    }

    return *path;
}

void ensure_data_paths_exist() {
    std::error_code ec;

    // clang-format off
    fs::create_directories(app_data_path(), ec);
    if (!ec) fs::create_directories(layouts_dir_path(), ec);
    // clang-format on

    if (ec)
        Logger::critical_error("App", "Failed to create app directories necessary for execution.");
}

} // namespace meta