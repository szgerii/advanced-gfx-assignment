#pragma once

#include <memory>
#include <vector>

#include "core/logger.h"
#include "gl/buffer.h"
#include "gl/sampler.h"
#include "gl/shader_program.h"
#include "gl/textures.h"
#include "vendor.h"

// this could be forced to use TypedBuffer directly either by
// a) introducing a base class and making bind_base virtual (tiny overhead everywhere)
// b) std::variant of all CCanBindBase-fulfilling types (small overhead in interface only)
// c) type erasure with a quasi-virtual wrapper around bind_base (tiny overhead in interface only)
// the TypedBuffer abstraction just didn't seem meaningful enough to bother with any of these and
// accept the overhead
// it could potentially be useful for introducing some debugging capabilities into ProgramInterface,
// which would enable quick debugging of wrongly set up interfaces, but that's a rare scenario too
// TL;DR: we immediately decay input TypedBuffers to their underlying handles in interfaces

struct BufferBinding {
    GLenum target;
    GLuint idx;
    GLuint buffer;

    DEFINE_VALIDATOR({
        assert(
            (target == GL_ATOMIC_COUNTER_BUFFER || target == GL_TRANSFORM_FEEDBACK_BUFFER ||
             target == GL_UNIFORM_BUFFER || target == GL_SHADER_STORAGE_BUFFER) &&
            "target is a non-index-bindable buffer type, or not a valid OpenGL buffer type at all");
        assert(buffer != 0 && "buffer binding without an \"attached\" buffer is not allowed");
    })
};

struct TextureBinding {
    GLuint unit;
    const Texture* texture;
    const Sampler* sampler; // optional

    DEFINE_VALIDATOR({
        assert(texture != nullptr && "texture binding without a texture is not allowed");
        assert(
            texture->id() != 0 &&
            "uninitalized, or moved-from texture used in active program interface texture binding");
        assert((sampler == nullptr || sampler->id() != 0) &&
               "uninitialized, or moved-from sampler used in active program interface texture "
               "binding");
    })
};

class ProgramInterface {
    static constexpr size_t EXPECTED_BUFFER_BINDINGS = 4;
    static constexpr size_t EXPECTED_TEX_BINDINGS    = 8;

    std::shared_ptr<ShaderProgram> program_;
    std::vector<BufferBinding> buffer_bindings_{};
    std::vector<TextureBinding> texture_bindings_{};

public:
    ProgramInterface() = default;

    explicit ProgramInterface(std::shared_ptr<ShaderProgram> program)
        : program_{std::move(program)} {
        buffer_bindings_.reserve(EXPECTED_BUFFER_BINDINGS);
        texture_bindings_.reserve(EXPECTED_TEX_BINDINGS);
    }

    template <GLenum BufferType>
    requires CCanBindBase<BufferType>
    void attach(GLuint binding_idx, const TypedBuffer<BufferType>& buffer) {
        VALIDATE(*this);

#ifndef NDEBUG
        using BuffTraits = buffer_type_traits<BufferType>;

        GLint idx_limit = 0;
        glGetIntegerv(BuffTraits::max_bindings_pname, &idx_limit);
        if (binding_idx >= static_cast<GLuint>(idx_limit)) {
            Logger::critical_error(
                "ProgramInterface",
                std::format(
                    "trying to bind buffer of type {} to binding index {}, but the largest "
                    "supported index on the current system is {}. Note that this is "
                    "a debug-only assertion, and will be silently forwarded to OpenGL in release "
                    "builds.",
                    BuffTraits::type_name, binding_idx, idx_limit - 1));
        }
#endif

        for (auto& binding : buffer_bindings_) {
            if (binding.target == BufferType && binding.idx == binding_idx) {
                binding.buffer = buffer.id();
                return;
            }
        }

        buffer_bindings_.emplace_back(BufferType, binding_idx, buffer.id());
    }

    void attach(GLuint unit, const Texture* texture, const Sampler* sampler = nullptr) {
        for (auto& binding : texture_bindings_) {
            if (binding.unit == unit) {
                binding.texture = texture;
                binding.sampler = sampler;
                return;
            }
        }

        texture_bindings_.emplace_back(unit, texture, sampler);
    }

    void bind() {
        VALIDATE(*this);

        program_->use();

        for (const auto& binding : buffer_bindings_)
            glBindBufferBase(binding.target, binding.idx, binding.buffer);

        for (const auto& binding : texture_bindings_) {
            VALIDATE(binding);

            binding.texture->bind(binding.unit);
            if (binding.sampler != nullptr)
                binding.sampler->bind(binding.unit);
            else
                Sampler::unbind(binding.unit);
        }
    }

    void unbind() {
        glUseProgram(0);

        for (const auto& binding : buffer_bindings_)
            glBindBufferBase(binding.target, binding.idx, 0);

        for (const auto& binding : texture_bindings_) {
            Texture::unbind(binding.unit);
            if (binding.sampler != nullptr)
                Sampler::unbind(binding.unit);
        }
    }

    ShaderProgram& program() {
        if (program_ == nullptr)
            Logger::critical_error("ProgramInterface", "underlying program is null!");
        return *program_;
    }

    const ShaderProgram& program() const {
        if (program_ == nullptr)
            Logger::critical_error("ProgramInterface", "underlying program is null!");
        return *program_;
    }

    DEFINE_VALIDATOR({
        VALIDATE_ALL(buffer_bindings_);
        assert(program_ != nullptr && "program interface without shader program");
        VALIDATE(*program_);
    })
};
