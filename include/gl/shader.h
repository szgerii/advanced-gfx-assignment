#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include "utils/validate.h"
#include "vendor.h"

class ShaderProgram;

namespace detail {

inline constexpr bool is_valid_shader_stage_type(GLenum stage_type) {
    return stage_type == GL_VERTEX_SHADER || stage_type == GL_FRAGMENT_SHADER ||
           stage_type == GL_GEOMETRY_SHADER || stage_type == GL_TESS_CONTROL_SHADER ||
           stage_type == GL_TESS_EVALUATION_SHADER || stage_type == GL_COMPUTE_SHADER ||
           stage_type == GL_TASK_SHADER_NV || stage_type == GL_MESH_SHADER_NV;
}

} // namespace detail

class Shader {
    GLuint id_ = 0;

public:
    GLenum stage_type;
    std::string src;

    // NOTE: when src_path is set, src should not be modified manually
    // it's technically safe to do so, but it will be overwritten internally before the change could
    // be reflected anywhere
    std::optional<std::filesystem::path> src_path;

    DEFINE_VALIDATOR({
        assert(detail::is_valid_shader_stage_type(stage_type) && "invalid shader stage type");
        assert(!src.empty() && "shader source code cannot be empty");
    })

    explicit Shader(GLenum stage_type, std::string src)
        : stage_type{stage_type}, src{std::move(src)} {
        compile();
    }

    explicit Shader(GLenum stage_type, std::filesystem::path src_path)
        : stage_type{stage_type}, src_path{src_path} {
        compile();
    }

    // factory to allow file import at call site without having to use std::filesystem::path
    // explicitly (string types can be implicitly converted to paths as args)
    static Shader from_file(GLenum stage_type, std::filesystem::path src_path) {
        return Shader{stage_type, src_path};
    }

    ~Shader() {
        if (id_ != 0)
            glDeleteShader(id_);
    }

    Shader(const Shader&)            = delete;
    Shader& operator=(const Shader&) = delete;

    Shader(Shader&& other) noexcept
        : id_{other.id_},
          stage_type{other.stage_type},
          src{std::move(other.src)},
          src_path{std::move(other.src_path)} {
        other.id_ = 0;
    }

    Shader& operator=(Shader&& other) noexcept {
        if (this == &other)
            return *this;

        this->stage_type = other.stage_type;
        this->src        = std::move(other.src);
        this->src_path   = std::move(other.src_path);

        this->id_ = other.id_;
        other.id_ = 0;

        return *this;
    }

    void compile(bool force_recreate = false);

    GLuint id() const { return id_; }

private:
    void load_src();
};
