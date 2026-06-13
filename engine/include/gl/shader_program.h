#pragma once

#include <algorithm>
#include <concepts>
#include <functional>
#include <iterator>
#include <memory>
#include <span>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "gl/shader.h"
#include "utils/transparent_string_hash.h"
#include "utils/validate.h"
#include "vendor.h"

class ShaderProgram;

struct ShaderStage {
    std::shared_ptr<Shader> shader;
    bool active; // controls whether a given shader object is attached to the program

private:
    friend class ShaderProgram;
    bool attached_ = false;

public:
    ShaderStage(std::shared_ptr<Shader> shader, bool active = true)
        : shader{std::move(shader)}, active{active} {}

    operator std::shared_ptr<Shader>() const { return shader; }

    bool is_attached() const { return attached_; }

    DEFINE_VALIDATOR({
        assert(shader != nullptr && "invalid Shader handle in ShaderStage in use by a program");
        VALIDATE(*shader);
    })
};

class ShaderProgram {
    GLuint program_id_{0};

    mutable std::unordered_map<std::string, GLint, TransparentStringHash, std::equal_to<>>
        uniform_cache_{};

public:
    std::vector<ShaderStage> stages{};

    DEFINE_VALIDATOR({
        assert(program_id_ != 0 && "shader program with uninitialized or moved-from handle");
        VALIDATE_ALL(stages);
    })

    explicit ShaderProgram(std::vector<ShaderStage> stages)
        : stages{std::move(stages)} {
        VALIDATE_ALL(stages);
    }

    template <typename... ShaderStages>
    requires (std::convertible_to<ShaderStages, ShaderStage> && ...)
    explicit ShaderProgram(ShaderStages&&... stages) {
        this->stages.reserve(sizeof...(stages));
        (this->stages.push_back(std::forward<ShaderStages>(stages)), ...);
        VALIDATE_ALL(this->stages);
    }

    explicit ShaderProgram(std::vector<std::shared_ptr<Shader>> shaders) {
        stages.reserve(shaders.size());
        std::transform(std::make_move_iterator(shaders.begin()),
                       std::make_move_iterator(shaders.end()), std::back_inserter(stages),
                       [](std::shared_ptr<Shader>&& obj) { return ShaderStage(std::move(obj)); });
        VALIDATE_ALL(stages);
    }

    template <typename... Shaders>
    requires (std::convertible_to<Shaders, Shader> && ...)
    explicit ShaderProgram(std::shared_ptr<Shaders>... shaders)
        : ShaderProgram{ShaderStage{std::move(shaders)}...} {}

    ~ShaderProgram() {
        if (program_id_ != 0)
            glDeleteProgram(program_id_);
    }

    ShaderProgram(const ShaderProgram&)            = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;

    ShaderProgram(ShaderProgram&& other) noexcept
        : program_id_{other.program_id_},
          uniform_cache_{std::move(other.uniform_cache_)},
          stages{std::move(other.stages)} {
        other.program_id_ = 0;
    }

    ShaderProgram& operator=(ShaderProgram&& other) noexcept {
        if (this == &other)
            return *this;

        if (program_id_ != 0)
            glDeleteProgram(program_id_);

        program_id_       = other.program_id_;
        other.program_id_ = 0;

        stages         = std::move(other.stages);
        uniform_cache_ = std::move(other.uniform_cache_);

        return *this;
    }

    // update attachments + (re)link program
    void link(bool force_recreate = false);

    // (re)compile shader objects + call link
    void build(bool force_recreate_program = false, bool force_recreate_shaders = false);

    void use() {
        VALIDATE(*this);
        glUseProgram(program_id_);
    }

    GLint uniform_location(std::string_view name) const;

    void load(std::string_view name) const;
    void load(const std::vector<std::string_view>& names) const;

    // overloads for any scalar or glm-backed uniform upload (incl. memory-contigous collections)
    // exception: bool and bvecn arrays (they would require a vector<int> alloc & init per upload)
    // just bypass them and use ints directly or something, idk

#define DEFINE_UPLOAD_UNIFORM(T, scalar)                                                           \
    void upload_uniform(std::string_view name, T value) {                                          \
        VALIDATE(*this);                                                                           \
        glProgramUniform1##scalar(program_id_, uniform_location(name), value);                     \
    }                                                                                              \
                                                                                                   \
    void upload_uniform(std::string_view name, std::span<const T> values) {                        \
        VALIDATE(*this);                                                                           \
        glProgramUniform1##scalar##v(program_id_, uniform_location(name),                          \
                                     static_cast<GLsizei>(values.size()),                          \
                                     !values.empty() ? values.data() : nullptr);                   \
    }

#define DEFINE_UPLOAD_UNIFORM_VECS(T, count, scalar, ScalarT)                                      \
    void upload_uniform(std::string_view name, T value) {                                          \
        VALIDATE(*this);                                                                           \
        glProgramUniform##count##scalar##v(program_id_, uniform_location(name), 1,                 \
                                           glm::value_ptr(value));                                 \
    }                                                                                              \
                                                                                                   \
    static_assert(sizeof(T) == sizeof(ScalarT) * count);                                           \
    void upload_uniform(std::string_view name, std::span<const T> values) {                        \
        VALIDATE(*this);                                                                           \
        glProgramUniform##count##scalar##v(                                                        \
            program_id_, uniform_location(name), static_cast<GLsizei>(values.size()),              \
            !values.empty() ? glm::value_ptr(values.front()) : nullptr);                           \
    }

#define DEFINE_UPLOAD_UNIFORM_MAT(TBase, scalar, N, M, dims, ScalarT)                              \
    void upload_uniform(std::string_view name, const TBase##dims& value) {                         \
        VALIDATE(*this);                                                                           \
        glProgramUniformMatrix##dims##scalar##v(program_id_, uniform_location(name), 1, GL_FALSE,  \
                                                glm::value_ptr(value));                            \
    }                                                                                              \
                                                                                                   \
    static_assert(sizeof(TBase##dims) == sizeof(ScalarT) * (N) * (M));                             \
    void upload_uniform(std::string_view name, std::span<const TBase##dims> values) {              \
        VALIDATE(*this);                                                                           \
        glProgramUniformMatrix##dims##scalar##v(                                                   \
            program_id_, uniform_location(name), static_cast<GLsizei>(values.size()), GL_FALSE,    \
            !values.empty() ? glm::value_ptr(values.front()) : nullptr);                           \
    }

#define DEFINE_UPLOAD_UNIFORM_MATS(TBase, scalar, ScalarT)                                         \
    DEFINE_UPLOAD_UNIFORM_MAT(TBase, scalar, 2, 3, 2x3, ScalarT)                                   \
    DEFINE_UPLOAD_UNIFORM_MAT(TBase, scalar, 3, 2, 3x2, ScalarT)                                   \
    DEFINE_UPLOAD_UNIFORM_MAT(TBase, scalar, 2, 4, 2x4, ScalarT)                                   \
    DEFINE_UPLOAD_UNIFORM_MAT(TBase, scalar, 4, 2, 4x2, ScalarT)                                   \
    DEFINE_UPLOAD_UNIFORM_MAT(TBase, scalar, 3, 4, 3x4, ScalarT)                                   \
    DEFINE_UPLOAD_UNIFORM_MAT(TBase, scalar, 4, 3, 4x3, ScalarT)                                   \
    DEFINE_UPLOAD_UNIFORM_MAT(TBase, scalar, 2, 2, 2, ScalarT)                                     \
    DEFINE_UPLOAD_UNIFORM_MAT(TBase, scalar, 3, 3, 3, ScalarT)                                     \
    DEFINE_UPLOAD_UNIFORM_MAT(TBase, scalar, 4, 4, 4, ScalarT)

    // clang-format off
    DEFINE_UPLOAD_UNIFORM(GLfloat,  f)
    DEFINE_UPLOAD_UNIFORM(GLint,    i)
    DEFINE_UPLOAD_UNIFORM(GLuint,   ui)
    DEFINE_UPLOAD_UNIFORM(GLdouble, d)

    DEFINE_UPLOAD_UNIFORM_VECS(glm::vec2,  2, f,  float)
    DEFINE_UPLOAD_UNIFORM_VECS(glm::vec3,  3, f,  float)
    DEFINE_UPLOAD_UNIFORM_VECS(glm::vec4,  4, f,  float)
    DEFINE_UPLOAD_UNIFORM_VECS(glm::ivec2, 2, i,  int)
    DEFINE_UPLOAD_UNIFORM_VECS(glm::ivec3, 3, i,  int)
    DEFINE_UPLOAD_UNIFORM_VECS(glm::ivec4, 4, i,  int)
    DEFINE_UPLOAD_UNIFORM_VECS(glm::uvec2, 2, ui, unsigned int)
    DEFINE_UPLOAD_UNIFORM_VECS(glm::uvec3, 3, ui, unsigned int)
    DEFINE_UPLOAD_UNIFORM_VECS(glm::uvec4, 4, ui, unsigned int)
    DEFINE_UPLOAD_UNIFORM_VECS(glm::dvec2, 2, d,  double)
    DEFINE_UPLOAD_UNIFORM_VECS(glm::dvec3, 3, d,  double)
    DEFINE_UPLOAD_UNIFORM_VECS(glm::dvec4, 4, d,  double)

    DEFINE_UPLOAD_UNIFORM_MATS(glm::mat,  f, float)
    DEFINE_UPLOAD_UNIFORM_MATS(glm::dmat, d, double)
    // clang-format on

    void upload_uniform(std::string_view name, bool value) {
        VALIDATE(*this);
        glProgramUniform1i(program_id_, uniform_location(name), value);
    }
    void upload_uniform(std::string_view name, glm::bvec2 value) {
        VALIDATE(*this);
        glProgramUniform2i(program_id_, uniform_location(name), value.x, value.y);
    }
    void upload_uniform(std::string_view name, glm::bvec3 value) {
        VALIDATE(*this);
        glProgramUniform3i(program_id_, uniform_location(name), value.x, value.y, value.z);
    }
    void upload_uniform(std::string_view name, glm::bvec4 value) {
        VALIDATE(*this);
        glProgramUniform4i(program_id_, uniform_location(name), value.x, value.y, value.z, value.w);
    }

#undef DEFINE_UPLOAD_UNIFORM_MAT
#undef DEFINE_UPLOAD_UNIFORM_MATS
#undef DEFINE_UPLOAD_UNIFORM_VECS
#undef DEFINE_UPLOAD_UNIFORM

private:
    void update_attachments();
};
