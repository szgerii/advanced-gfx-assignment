#pragma once

#include <cstdint>
#include <filesystem>
#include <format>
#include <span>

#include "core/logger.h"
#include "vendor.h"

class ImageRGBA {
    using Texel = glm::u8vec4;
    static_assert(sizeof(Texel) == sizeof(uint32_t));

    std::vector<Texel> texels_{};
    uint32_t width_ = 0, height_ = 0;

public:
    void alloc(uint32_t width, uint32_t height) {
        width_  = width;
        height_ = height;

        texels_.resize(width * height);
    }

    void assign(std::span<const Texel> data, uint32_t width, uint32_t height) {
        width_  = width;
        height_ = height;

        if (data.size() < width * height) {
            Logger::critical_error(
                "ImageRGBA",
                std::format("Insufficient data provided for image texel data assignment "
                            "(expected: {} texels for grid {}x{}, got: {})",
                            width * height, width, height, data.size()));
        }

        texels_.assign(data.begin(), data.end());
    }

    Texel texel_at(size_t idx) const { return texels_.at(idx); }
    Texel texel_at(size_t x, size_t y) const { return texels_.at(y * width_ + x); }

    void set_texel(size_t idx, Texel texel) { texels_.at(idx) = texel; }
    void set_texel(size_t x, size_t y, Texel texel) { texels_.at(y * width_ + x) = texel; }

    const std::vector<Texel>& texels() const { return texels_; }
    uint32_t width() const { return width_; }
    uint32_t height() const { return height_; }

    void load(const std::filesystem::path& img_path, bool needs_flip = true);

    static ImageRGBA from_file(const std::filesystem::path& img_path, bool needs_flip = true) {
        ImageRGBA img{};
        img.load(img_path, needs_flip);
        return img;
    }
};
