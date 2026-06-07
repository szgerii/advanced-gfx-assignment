#pragma once

#include <ranges>
#include <span>
#include <type_traits>

#include "gl/buffer_type_traits.h"
#include "logger.h"
#include "utils/num_cast.h"
#include "vendor.h"

template <GLenum BufferType>
requires CIsBufferType<BufferType>
class TypedBuffer {
    GLuint id_   = 0;
    size_t size_ = 0;

public:
    explicit TypedBuffer(size_t size, const void* data = nullptr,
                         GLbitfield flags = GL_DYNAMIC_STORAGE_BIT)
        : size_{size} {
        glCreateBuffers(1, &id_);
        glNamedBufferStorage(id_, num_cast<GLsizeiptr>(size), data, flags);
    }

    template <std::ranges::contiguous_range R>
    requires std::is_trivially_copyable_v<std::ranges::range_value_t<R>>
    explicit TypedBuffer(const R& container, GLbitfield flags = GL_DYNAMIC_STORAGE_BIT) {
        auto span = std::span{container};

        size_ = span.size_bytes();
        glCreateBuffers(1, &id_);
        glNamedBufferStorage(id_, num_cast<GLsizeiptr>(size_), span.data(), flags);
    }

    ~TypedBuffer() {
        if (id_ != 0)
            glDeleteBuffers(1, &id_);
    }

    TypedBuffer(const TypedBuffer&)            = delete;
    TypedBuffer& operator=(const TypedBuffer&) = delete;

    TypedBuffer(TypedBuffer&& other) noexcept
        : id_{other.id_}, size_{other.size_} {
        other.id_ = 0;
    }

    TypedBuffer& operator=(TypedBuffer&& other) noexcept {
        if (this == &other)
            return *this;

        if (id_ != 0)
            glDeleteBuffers(1, &id_);

        id_   = other.id_;
        size_ = other.size_;

        other.id_ = 0;

        return *this;
    }

    template <typename T>
    requires std::is_trivially_copyable_v<T> && (!std::ranges::range<T>)
    void upload(const T& data, GLintptr offset = 0) {
        assert_no_buffer_overflow(sizeof(T), offset, false);
        glNamedBufferSubData(id_, offset, static_cast<GLsizeiptr>(sizeof(T)), &data);
    }

    template <std::ranges::contiguous_range R>
    requires std::is_trivially_copyable_v<std::ranges::range_value_t<R>>
    void upload(const R& container, GLintptr offset = 0) {
        auto span = std::span{container};

        assert_no_buffer_overflow(span.size_bytes(), offset, false);
        glNamedBufferSubData(id_, offset, static_cast<GLsizeiptr>(span.size_bytes()), span.data());
    }

    // prefer bind(), only use if you explicitly wanna bypass the Typed part of TypedBuffer
    void bind_to_target(GLenum target) { glBindBuffer(target, id_); }

    void bind() { bind_to_target(BufferType); }

    void bind_base(GLuint idx) requires CCanBindBase<BufferType>
    {
        glBindBufferBase(BufferType, idx, id_);
    }

    void bind_range(GLuint idx, GLintptr offset, GLsizeiptr size) requires CCanBindBase<BufferType>
    {
        assert_no_buffer_overflow(static_cast<size_t>(size), offset, true);
        glBindBufferRange(BufferType, idx, id_, offset, size);
    }

    void unbind() { glBindBuffer(BufferType, 0); }
    void unbind_base(GLuint idx) { glBindBufferBase(BufferType, idx, 0); }

    void* map(GLenum access) { return glMapNamedBuffer(id_, access); }
    bool unmap() { return glUnmapNamedBuffer(id_) == GL_TRUE; }

    GLuint id() const { return id_; }
    size_t size() const { return size_; }

private:
    void assert_no_buffer_overflow(size_t data_size, GLintptr offset, bool read) {
#ifndef NDEBUG
        if (static_cast<size_t>(offset) + data_size > size_) {
            Logger::critical_error(
                "Buffer",
                std::format(
                    "trying to {} {} bytes {} buffer starting at offset {}, which "
                    "would overflow the buffer's storage (buffer is {} bytes). Note that this is "
                    "a debug-only assertion, and will be silently forwarded to OpenGL in release "
                    "builds.",
                    read ? "access" : "upload", data_size, read ? "from" : "to", offset, size_));
        }
#endif
    }
};

using UniformBuffer       = TypedBuffer<GL_UNIFORM_BUFFER>;
using VertexBuffer        = TypedBuffer<GL_ARRAY_BUFFER>;
using IndexBuffer         = TypedBuffer<GL_ELEMENT_ARRAY_BUFFER>;
using ShaderStorageBuffer = TypedBuffer<GL_SHADER_STORAGE_BUFFER>;
