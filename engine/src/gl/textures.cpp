#include "gl/textures.h"

#include <stdexcept>

#include "render/ogl.h"

namespace {

// returned size is not guaranteed to be accurate if type and format are incompatible
// use alignment of GL_UNPACK_ALIGNMENT for host -> device (see glPixelStore, 4 by default)
// use alignment of GL_PACK_ALIGNMENT for device -> host (see glPixelStore, 4 by default)
// though the math works with any alignment, it's restricted to be either 1/2/4/8 by OpenGL
constexpr std::optional<size_t> tex_upload_expected_size(GLenum format, GLenum type, GLsizei width,
                                                         GLsizei height, GLsizei depth = 1,
                                                         uint32_t alignment = 4) {
    if (width < 0 || height < 0)
        return std::nullopt;
    if (width == 0 || height == 0)
        return 0;

    size_t bytes_per_pixel = 0;
    bool is_unpacked       = false;

    // packed types
    switch (type) {
        case GL_UNSIGNED_BYTE_3_3_2:
        case GL_UNSIGNED_BYTE_2_3_3_REV:
            bytes_per_pixel = 1;
            break;

        case GL_UNSIGNED_SHORT_5_6_5:
        case GL_UNSIGNED_SHORT_5_6_5_REV:
        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_4_4_4_4_REV:
        case GL_UNSIGNED_SHORT_5_5_5_1:
        case GL_UNSIGNED_SHORT_1_5_5_5_REV:
            bytes_per_pixel = 2;
            break;

        case GL_UNSIGNED_INT_8_8_8_8:
        case GL_UNSIGNED_INT_8_8_8_8_REV:
        case GL_UNSIGNED_INT_10_10_10_2:
        case GL_UNSIGNED_INT_2_10_10_10_REV:
            bytes_per_pixel = 4;
            break;

        default:
            is_unpacked = true;
            break;
    }

    // unpacked types (bytes per pixel == num of components * bytes per component)
    if (is_unpacked) {
        size_t component_count = 0, component_size = 0;

        switch (format) {
            case GL_RED:
            case GL_DEPTH_COMPONENT:
            case GL_STENCIL_INDEX:
                component_count = 1;
                break;

            case GL_RG:
                component_count = 2;
                break;

            case GL_RGB:
            case GL_BGR:
                component_count = 3;
                break;

            case GL_RGBA:
            case GL_BGRA:
                component_count = 4;
                break;

            default:
                return std::nullopt;
        }

        switch (type) {
            case GL_UNSIGNED_BYTE:
            case GL_BYTE:
                component_size = 1;
                break;

            case GL_UNSIGNED_SHORT:
            case GL_SHORT:
                component_size = 2;
                break;

            case GL_UNSIGNED_INT:
            case GL_INT:
            case GL_FLOAT:
                component_size = 4;
                break;

            default:
                return std::nullopt;
        }

        bytes_per_pixel = component_count * component_size;
    }

    size_t unaligned_row = bytes_per_pixel * num_cast<size_t>(width);
    size_t alignment_s   = num_cast<size_t>(alignment);
    size_t aligned_row   = (unaligned_row + alignment_s - 1) / alignment_s * alignment_s;

    return aligned_row * num_cast<size_t>(height) * static_cast<size_t>(depth);
}

void buffer_bounds_check(std::optional<size_t> expected_size, size_t actual_size,
                         bool intended_truncation = false) {
    if (expected_size.has_value()) {
        if (actual_size < expected_size) {
            Logger::critical_error(
                "Texture",
                std::format("Pixel data passed to texture upload is smaller than what is "
                            "expected for the provided format/type/dims combination (required "
                            "size in bytes: {}, got: {})",
                            *expected_size, actual_size));
        } else if (!intended_truncation && actual_size > *expected_size) {
            Logger::warn(
                "Texture",
                std::format(
                    "Pixel data passed to texture upload exceeds what is expected for "
                    "the provided format/type/dims combination (required size in bytes: {}, "
                    "got: {}). This might be intentional, in which case you can "
                    "silence this warning by setting upload's intended_truncation "
                    "argument to true (or, if it's a recurring problem, you can simply change "
                    "the default value in textures.h).",
                    *expected_size, actual_size));
        }
    } else {
        Logger::warn("Texture",
                     "Couldn't calculate expected pixel data size for texture "
                     "upload, bounds checking will not be performed. Note that this is a debug "
                     "mode only check and will not be performed in release builds.");
    }
}

} // namespace

#define TEX_PARAM_IMPL(pname, penum, ptype, suffix)                                                \
    void SampledTexture::set_##pname(ptype pname) {                                                \
        sample_params.pname = (pname);                                                             \
        glTextureParameter##suffix(id_, (penum), (pname));                                         \
    }

#define TEX_PARAM_IMPL_INT(pname, penum) TEX_PARAM_IMPL(pname, penum, GLint, i)
#define TEX_PARAM_IMPL_FLOAT(pname, penum) TEX_PARAM_IMPL(pname, penum, GLfloat, f)

TEX_PARAM_IMPL_INT(mipmap_base_level, GL_TEXTURE_BASE_LEVEL)
TEX_PARAM_IMPL_INT(mipmap_max_level, GL_TEXTURE_MAX_LEVEL)
TEX_PARAM_IMPL_INT(min_filter, GL_TEXTURE_MIN_FILTER)
TEX_PARAM_IMPL_INT(mag_filter, GL_TEXTURE_MAG_FILTER)
TEX_PARAM_IMPL_INT(wrap_s, GL_TEXTURE_WRAP_S)
TEX_PARAM_IMPL_INT(wrap_t, GL_TEXTURE_WRAP_T)
TEX_PARAM_IMPL_INT(wrap_r, GL_TEXTURE_WRAP_R)
TEX_PARAM_IMPL_INT(min_lod, GL_TEXTURE_MIN_LOD)
TEX_PARAM_IMPL_INT(max_lod, GL_TEXTURE_MAX_LOD)
TEX_PARAM_IMPL_FLOAT(lod_bias, GL_TEXTURE_LOD_BIAS)
TEX_PARAM_IMPL_INT(depth_stencil_mode, GL_DEPTH_STENCIL_TEXTURE_MODE)
TEX_PARAM_IMPL_INT(compare_func, GL_TEXTURE_COMPARE_FUNC)
TEX_PARAM_IMPL_INT(compare_mode, GL_TEXTURE_COMPARE_MODE)

#undef TEX_PARAM_IMPL_FLOAT
#undef TEX_PARAM_IMPL_INT
#undef TEX_PARAM_IMPL

void SampledTexture::set_swizzle_r(GLint r) {
    sample_params.swizzle.r = r;
    glTextureParameteri(id_, GL_TEXTURE_SWIZZLE_R, r);
}

void SampledTexture::set_swizzle_g(GLint g) {
    sample_params.swizzle.g = g;
    glTextureParameteri(id_, GL_TEXTURE_SWIZZLE_G, g);
}

void SampledTexture::set_swizzle_b(GLint b) {
    sample_params.swizzle.b = b;
    glTextureParameteri(id_, GL_TEXTURE_SWIZZLE_B, b);
}

void SampledTexture::set_swizzle_a(GLint a) {
    sample_params.swizzle.a = a;
    glTextureParameteri(id_, GL_TEXTURE_SWIZZLE_A, a);
}

void SampledTexture::set_swizzle_rgba(glm::ivec4 rgba) {
    sample_params.swizzle = rgba;
    glTextureParameteriv(id_, GL_TEXTURE_SWIZZLE_RGBA, glm::value_ptr(rgba));
}

void SampledTexture::set_border_color(glm::vec4 border_color) {
    sample_params.border_color = border_color;
    glTextureParameterfv(id_, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(border_color));
}

void SampledTexture::set_swizzle(glm::ivec4 rgba) {
    set_swizzle_rgba(rgba);
}
void SampledTexture::set_swizzle(GLint r, GLint g, GLint b, GLint a) {
    set_swizzle_rgba({r, g, b, a});
}

void SampledTexture::set_mipmap_levels(GLint base_level, GLint max_level) {
    set_mipmap_base_level(base_level);
    set_mipmap_max_level(max_level);
}

void SampledTexture::set_filters(GLint min_filter, GLint mag_filter) {
    set_min_filter(min_filter);
    set_mag_filter(mag_filter);
}

void SampledTexture::set_wrap(GLint s, GLint t) {
    set_wrap_s(s);
    set_wrap_t(t);
}

void SampledTexture::set_wrap(GLint s, GLint t, GLint r) {
    set_wrap(s, t);
    set_wrap_r(r);
}

void SampledTexture::set_lod(GLint min_lod, GLint max_lod, GLfloat lod_bias) {
    set_min_lod(min_lod);
    set_max_lod(max_lod);
    set_lod_bias(lod_bias);
}

void SampledTexture::set_depth(GLint depth_stencil_mode, GLint compare_func, GLint compare_mode) {
    set_depth_stencil_mode(depth_stencil_mode);
    set_compare_func(compare_func);
    set_compare_mode(compare_mode);
}

void SampledTexture::update_parameters() {
    // ehh, but works
    set_mipmap_levels(sample_params.mipmap_base_level, sample_params.mipmap_max_level);
    set_filters(sample_params.min_filter, sample_params.mag_filter);
    set_wrap(sample_params.wrap_s, sample_params.wrap_t, sample_params.wrap_r);
    set_lod(sample_params.min_lod, sample_params.max_lod, sample_params.lod_bias);
    set_depth(sample_params.depth_stencil_mode, sample_params.compare_func,
              sample_params.compare_mode);
    set_swizzle(sample_params.swizzle);
    set_border_color(sample_params.border_color);
}

// source: lab code
GLint SampledTexture::calculate_mip_levels(GLint width, GLint height) {
    GLint result = 1;
    GLint index  = std::max(width, height);

    while (index >>= 1)
        result++;

    return result;
}

void Texture2D::upload(std::span<const uint8_t> pixels, GLint x_offset, GLint y_offset, GLint width,
                       GLint height, GLint level, GLenum data_format, GLenum data_type,
                       bool intended_truncation) {
#ifndef NDEBUG
    // check if pixels buffer matches size expected by OpenGL
    auto expected_size =
        tex_upload_expected_size(data_format, data_type, width, height, 1, OGL::unpack_alignment());
    buffer_bounds_check(expected_size, pixels.size_bytes(), intended_truncation);
#endif

    glTextureSubImage2D(id_, level, x_offset, y_offset, width, height, data_format, data_type,
                        pixels.data());
}

void TextureCube::upload(std::span<const uint8_t> pixels, GLint x_offset, GLint y_offset,
                         GLint z_offset, GLint width, GLint height, GLint depth, GLint level,
                         GLenum data_format, GLenum data_type, bool intended_truncation) {
#ifndef NDEBUG
    // check if pixels buffer matches size expected by OpenGL
    auto expected_size = tex_upload_expected_size(data_format, data_type, width, height, depth,
                                                  OGL::unpack_alignment());
    buffer_bounds_check(expected_size, pixels.size_bytes(), intended_truncation);
#endif

    glTextureSubImage3D(id_, level, x_offset, y_offset, z_offset, width, height, depth, data_format,
                        data_type, pixels.data());
}
