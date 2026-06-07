#pragma once

#include <memory>
#include <vector>

#include "gl/buffer.h"
#include "gl/shader_program.h"
#include "logger.h"
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

class ProgramInterface {
    static constexpr size_t EXPECTED_BINDING_COUNT = 4;

    std::shared_ptr<ShaderProgram> program_;
    std::vector<BufferBinding> binding_table_{};

public:
    explicit ProgramInterface(std::shared_ptr<ShaderProgram> program)
        : program_{std::move(program)} {
        binding_table_.reserve(EXPECTED_BINDING_COUNT);
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

        for (auto& binding : binding_table_) {
            if (binding.target == BufferType && binding.idx == binding_idx) {
                binding.buffer = buffer.id();
                return;
            }
        }

        binding_table_.emplace_back(BufferType, binding_idx, buffer.id());
    }

    void bind() {
        VALIDATE(*this);

        program_->use();

        for (const auto& binding : binding_table_)
            glBindBufferBase(binding.target, binding.idx, binding.buffer);
    }

    void unbind() {
        glUseProgram(0);

        for (const auto& binding : binding_table_)
            glBindBufferBase(binding.target, binding.idx, 0);
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
        VALIDATE_ALL(binding_table_);
        assert(program_ != nullptr && "program interface without shader program");
        VALIDATE(*program_);
    })
};
