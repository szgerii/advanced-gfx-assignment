#pragma once

#include "gl/textures.h"
#include "vendor.h"

class Sampler {
    GLuint id_ = 0;

public:
    // TextureParams fields that can only be used on textures will be ignored
    TextureParams params;

    explicit Sampler(TextureParams params = {})
        : params{params} {
        glCreateSamplers(1, &id_);
        update_parameters();
    }

    ~Sampler() {
        if (id_ != 0)
            glDeleteSamplers(1, &id_);
    }

    Sampler(const Sampler&)            = delete;
    Sampler& operator=(const Sampler&) = delete;

    Sampler(Sampler&& other) noexcept
        : id_{other.id_}, params{other.params} {
        other.id_ = 0;
    }

    Sampler& operator=(Sampler&& other) noexcept {
        if (this == &other)
            return *this;

        if (id_ != 0)
            glDeleteSamplers(1, &id_);

        id_       = other.id_;
        other.id_ = 0;
        params    = other.params;

        return *this;
    }

    void set_min_filter(GLint min_filter);
    void set_mag_filter(GLint mag_filter);
    void set_wrap_s(GLint wrap_s);
    void set_wrap_t(GLint wrap_t);
    void set_wrap_r(GLint wrap_r);
    void set_min_lod(GLint min_lod);
    void set_max_lod(GLint max_lod);
    void set_lod_bias(GLfloat lod_bias);
    void set_compare_func(GLint compare_func);
    void set_compare_mode(GLint compare_mode);
    void set_border_color(glm::vec4 border_color);

    void set_filters(GLint min_filter, GLint mag_filter);
    void set_wrap(GLint s, GLint t);
    void set_wrap(GLint s, GLint t, GLint r);
    void set_lod(GLint min_lod, GLint max_lod, GLfloat lod_bias);
    void set_depth(GLint compare_func, GLint compare_mode);

    void update_parameters();

    void bind(GLuint texture_unit) { glBindSampler(texture_unit, id_); }
    void unbind(GLuint texture_unit) { glBindSampler(texture_unit, 0); }

    GLuint id() const { return id_; }
};
