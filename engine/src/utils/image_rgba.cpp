#include "utils/image_rgba.h"

#include <format>

#include "core/logger.h"
#include "stb_image.h"
#include "utils/num_cast.h"

void ImageRGBA::load(const std::filesystem::path& img_path, bool needs_flip) {
    stbi_set_flip_vertically_on_load(needs_flip);

    int w = 0, h = 0, channels = 0;

    stbi_uc* data = stbi_load(img_path.string().c_str(), &w, &h, &channels, STBI_rgb_alpha);

    if (data == nullptr) {
        Logger::critical_error("ImageRGBA", std::format("Failed to load image file '{}':\n{}",
                                                        img_path.string(), stbi_failure_reason()));
    }

    size_t texel_count = num_cast<size_t>(w) * num_cast<size_t>(h);
    assign(std::span<const Texel>(reinterpret_cast<const Texel*>(data), texel_count),
           num_cast<uint32_t>(w), num_cast<uint32_t>(h));

    stbi_image_free(data);
}