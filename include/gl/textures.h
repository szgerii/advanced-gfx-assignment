#pragma once

#include <cstdint>
#include <format>
#include <optional>
#include <span>

#include "logger.h"
#include "render/ogl.h"
#include "vendor.h"

// hypothesis: even if we flat store a list of Texture-s later, we won't need to be able to do
// anything else other than call bind and move/destruct through a generic texture handle
// everything else (data upload, sampling config, etc.) should be handled via the derived types
// that means, we can make the whole thing a proper inheritance chain, where each subclass allows
// initialization and upload capabilities via an easy to use interface, but only the destructor
// needs to become virtual in the base class, so: good perf, dry code
// if this comment is still here, that worked out, or i forgot to remove it. unlucky

// a single "opaque" texture handle wrapper RAII
class Texture {
protected:
    // used by all textures as defaults
    static constexpr GLenum DEFAULT_INTERNAL_FORMAT = GL_RGBA8;
    static constexpr GLenum DEFAULT_DATA_FORMAT     = GL_RGBA;
    static constexpr GLenum DEFAULT_DATA_TYPE       = GL_UNSIGNED_BYTE;

    GLuint id_ = 0;
    GLenum target_;
    GLenum internal_format_;

public:
    explicit Texture(GLenum target, GLenum internal_format = DEFAULT_INTERNAL_FORMAT)
        : target_{target}, internal_format_{internal_format} {
        glCreateTextures(target, 1, &id_);
    }

    // only the dtor is virtual, bind/setters/uploads are all static
    // (thus, this can only ever be a bottleneck if textures are constantly being discard [don't do
    // that tho])
    virtual ~Texture() {
        if (id_ != 0)
            glDeleteTextures(1, &id_);
    }

    Texture(const Texture&)            = delete;
    Texture& operator=(const Texture&) = delete;

    Texture(Texture&& other) noexcept
        : id_{other.id_} {
        other.id_ = 0;
    }

    Texture& operator=(Texture&& other) noexcept {
        if (this == &other)
            return *this;

        if (id_ != 0)
            glDeleteTextures(1, &id_);

        id_       = other.id_;
        other.id_ = 0;

        return *this;
    }

    // maybe bind could be a bit more specialized based on target_, if needed
    void bind(GLuint texture_unit) const { glBindTextureUnit(texture_unit, id_); }
    void unbind(GLuint texture_unit) const { glBindTextureUnit(texture_unit, 0); }

    GLuint id() const { return id_; }
    GLenum target() const { return target_; }
    GLenum internal_format() const { return internal_format_; }
};

// wherever specified, initial values copied from
// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexParameter.xhtml
// for other properties reasonable default values have been selected
// it does mean that texture "overwrite" default values even if not used, but i think it's better to
// keep data centralized CPU-side (without relying on std::optional gimmicks for upload)
struct TextureParams {
    // filtering, mipmaps
    GLint mipmap_base_level = 0;
    GLint mipmap_max_level  = 1000;
    GLint min_filter        = GL_NEAREST_MIPMAP_LINEAR;
    GLint mag_filter        = GL_LINEAR;
    GLint wrap_s            = GL_REPEAT;
    GLint wrap_t            = GL_REPEAT;
    GLint wrap_r            = GL_REPEAT;

    // lod
    GLint min_lod    = -1000;
    GLint max_lod    = 1000;
    GLfloat lod_bias = 0.0f;

    // depth
    GLint depth_stencil_mode = GL_DEPTH_COMPONENT;
    GLint compare_func       = GL_LEQUAL;
    GLint compare_mode       = GL_NONE;

    // misc
    glm::ivec4 swizzle{GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA};
    glm::vec4 border_color{0.f};

    TextureParams() = default;

private:
    // helper ctor so that TextureCube can specify custom default wrapping
    // without having to specify all other default types twice
    friend class TextureCube;

    explicit TextureParams(GLint wrap_s, GLint wrap_t, GLint wrap_r)
        : wrap_s{wrap_s}, wrap_t{wrap_t}, wrap_r{wrap_r} {}

    explicit TextureParams(GLint wrap_str)
        : TextureParams{wrap_str, wrap_str, wrap_str} {}
};

// texture resource with interface for general sampling configuration
// mipmaps, filtering, depth, LOD, etc.
class SampledTexture : public Texture {
protected:
    GLint width_, height_;
    GLint mip_levels_;

public:
    // requires call to update_parameters() to refresh the actual texture's parameters
    // this also means it shouldn't be used for querying the parameters of a texture, unless they
    // are never updated without an immediate synchronization afterwards (and individual setters are
    // avoided)
    // prefer the use of samplers instead
    TextureParams sample_params{};

    // set mip_levels to 0 to automatically calculate
    explicit SampledTexture(GLenum target, GLint width, GLint height,
                            GLenum internal_format = DEFAULT_INTERNAL_FORMAT, GLint mip_levels = 0,
                            const TextureParams& sample_params = {})
        : Texture{target, internal_format},
          width_{width},
          height_{height},
          mip_levels_{mip_levels},
          sample_params{sample_params} {

        if (mip_levels_ == 0)
            mip_levels_ = calculate_mip_levels(width_, height_);

        update_parameters();
    }

    SampledTexture(SampledTexture&& other) noexcept
        : Texture{std::move(other)},
          width_{other.width_},
          height_{other.height_},
          mip_levels_{other.mip_levels_},
          sample_params{other.sample_params} {
        other.id_ = 0;
    }

    SampledTexture& operator=(SampledTexture&& other) noexcept {
        if (this == &other)
            return *this;

        Texture::operator=(std::move(other));
        width_        = other.width_;
        height_       = other.height_;
        mip_levels_   = other.mip_levels_;
        sample_params = other.sample_params;

        return *this;
    }

    GLint width() const { return width_; }
    GLint height() const { return height_; }
    GLint mip_levels() const { return mip_levels_; }
    void generate_mipmaps() { glGenerateTextureMipmap(id_); }

    // configures the underlying texture resource according to the current state of parameters
    void update_parameters();

    // explicit 1:1 setters
    // these setters automatically set parameters and update the internal resource
    // they're kinda cumbersome and bloated, but the intention was to keep gl calls abstracted,
    // while not forcing unnecessary modifications or explicit current value specifications at the
    // call-site
    void set_mipmap_base_level(GLint mipmap_base_level);
    void set_mipmap_max_level(GLint mipmap_max_level);
    void set_min_filter(GLint min_filter);
    void set_mag_filter(GLint mag_filter);
    void set_wrap_s(GLint wrap_s);
    void set_wrap_t(GLint wrap_t);
    void set_wrap_r(GLint wrap_r);
    void set_min_lod(GLint min_lod);
    void set_max_lod(GLint max_lod);
    void set_lod_bias(GLfloat lod_bias);
    void set_depth_stencil_mode(GLint depth_stencil_mode);
    void set_compare_func(GLint compare_func);
    void set_compare_mode(GLint compare_mode);
    void set_swizzle_r(GLint r);
    void set_swizzle_g(GLint g);
    void set_swizzle_b(GLint b);
    void set_swizzle_a(GLint a);
    void set_swizzle_rgba(glm::ivec4 rgba);
    void set_border_color(glm::vec4 border_color);

    // aggregate and alias setters
    void set_swizzle(glm::ivec4 rgba);
    void set_swizzle(GLint r, GLint g, GLint b, GLint a);

    void set_mipmap_levels(GLint base_level, GLint max_level);
    void set_filters(GLint min_filter, GLint mag_filter);
    void set_wrap(GLint s, GLint t);
    void set_wrap(GLint s, GLint t, GLint r);
    void set_lod(GLint min_lod, GLint max_lod, GLfloat lod_bias);
    void set_depth(GLint depth_stencil_mode, GLint compare_func, GLint compare_mode);

protected:
    static GLint calculate_mip_levels(GLint width, GLint height);
};

class Texture2D final : public SampledTexture {
public:
    explicit Texture2D(GLint width, GLint height, GLenum internal_format = DEFAULT_INTERNAL_FORMAT,
                       GLint mip_levels = 0, const TextureParams& sample_params = {})
        : SampledTexture{GL_TEXTURE_2D, width, height, internal_format, mip_levels, sample_params} {

        glTextureStorage2D(id_, mip_levels_, internal_format_, width_, height_);
    }

    // upload + mipmap gen ctor
    explicit Texture2D(GLint width, GLint height, std::span<const uint8_t> pixels,
                       GLenum data_format     = DEFAULT_DATA_FORMAT,
                       GLenum data_type       = DEFAULT_DATA_TYPE,
                       GLenum internal_format = DEFAULT_INTERNAL_FORMAT, GLint mip_levels = 0,
                       TextureParams sample_params = {}, bool intended_truncation = false)
        : Texture2D{width, height, internal_format, mip_levels, sample_params} {

        upload(pixels, data_format, data_type, intended_truncation);
        generate_mipmaps();
    }

    void upload(std::span<const uint8_t> pixels, GLint x_offset, GLint y_offset, GLint width,
                GLint height, GLint level = 0, GLenum data_format = DEFAULT_DATA_FORMAT,
                GLenum data_type = DEFAULT_DATA_TYPE, bool intended_truncation = false);

    void upload(std::span<const uint8_t> pixels, GLenum data_format = DEFAULT_DATA_FORMAT,
                GLenum data_type = DEFAULT_DATA_TYPE, bool intended_truncation = false) {
        upload(pixels, 0, 0, width_, height_, 0, data_format, data_type, intended_truncation);
    }
};

// enum matches the corresponding mipmap layer level
// https://wikis.khronos.org/opengl/Cubemap_Texture#Upload_and_orientation
enum class CubeFace : uint8_t {
    Right = 0, // X+
    Left,      // X-
    Top,       // Y+
    Bottom,    // Y-
    Front,     // Z+
    Back       // Z-
};

class TextureCube final : public SampledTexture {
    // class invariant: width_ == height_
public:
    explicit TextureCube(GLint size, GLenum internal_format = DEFAULT_INTERNAL_FORMAT,
                         GLint mip_levels                   = 0,
                         const TextureParams& sample_params = TextureParams{GL_CLAMP_TO_EDGE})
        : SampledTexture{GL_TEXTURE_CUBE_MAP, size,       size,
                         internal_format,     mip_levels, sample_params} {

        glTextureStorage2D(id_, mip_levels_, internal_format_, size, size);
    }

    // general upload
    void upload(std::span<const uint8_t> pixels, GLint x_offset, GLint y_offset, GLint z_offset,
                GLint width, GLint height, GLint depth, GLint level = 0,
                GLenum data_format = DEFAULT_DATA_FORMAT, GLenum data_type = DEFAULT_DATA_TYPE,
                bool intended_truncation = false);

    // for non-contigous (per-face fragmented) upload
    void upload_face(CubeFace face, std::span<const uint8_t> pixels, GLint x_offset, GLint y_offset,
                     GLint width, GLint height, GLint level = 0,
                     GLenum data_format = DEFAULT_DATA_FORMAT, GLenum data_type = DEFAULT_DATA_TYPE,
                     bool intended_truncation = false) {
        upload(pixels, x_offset, y_offset, static_cast<GLint>(face), width, height, 1, level,
               data_format, data_type, intended_truncation);
    }

    void upload_face(CubeFace face, std::span<const uint8_t> pixels,
                     GLenum data_format = DEFAULT_DATA_FORMAT, GLenum data_type = DEFAULT_DATA_TYPE,
                     bool intended_truncation = false) {
        upload_face(face, pixels, 0, 0, width_, height_, 0, data_format, data_type,
                    intended_truncation);
    }

    // for uploading all 6 faces at once
    void upload(std::span<const uint8_t> pixels, GLenum data_format = DEFAULT_DATA_FORMAT,
                GLenum data_type = DEFAULT_DATA_TYPE, bool intended_truncation = false) {
        upload(pixels, 0, 0, 0, width_, height_, 6, 0, data_format, data_type, intended_truncation);
    }

    GLint size() const { return width_; }
};

// TODO: Texture2DArray, Texture2DMultisample, ...
