#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace gui {

class LayoutManager {
private:
    // keep lifetime alive until next change
    inline static std::string ini_filename_buffer{};

public:
    static void try_load_layout(std::string_view name);
    static void try_save_layout_as(std::string_view name);

    static bool layout_exists(std::string_view name);

    static const std::vector<std::string>& get_layout_names(std::optional<float> current_total_ms,
                                                            bool force_reload = false);
};

} // namespace gui
