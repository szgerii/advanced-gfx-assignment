#pragma once

#include "core/logger.h"
#include "gl/buffer.h"

#include <format>

enum class AttribClass : uint8_t {
    Float,   // <==> glVertexArrayAttribFormat
    Integer, // <==> glVertexArrayAttribIFormat
    Double   // <==> glVertexArrayAttribLFormat
};

class VertexArray {
    GLuint id_ = 0;

public:
    explicit VertexArray() { glCreateVertexArrays(1, &id_); }

    ~VertexArray() {
        if (id_ != 0)
            glDeleteVertexArrays(1, &id_);
    }

    VertexArray(const VertexArray&)            = delete;
    VertexArray& operator=(const VertexArray&) = delete;

    VertexArray(VertexArray&& other)
        : id_{other.id_} {
        other.id_ = 0;
    }

    VertexArray& operator=(VertexArray&& other) {
        if (this == &other)
            return *this;

        if (id_ != 0)
            glDeleteVertexArrays(1, &id_);

        id_ = other.id_;

        other.id_ = 0;

        return *this;
    }

    void attach(const VertexBuffer& vbo, GLuint binding_idx, GLintptr offset, GLsizei stride) {
        glVertexArrayVertexBuffer(id_, binding_idx, vbo.id(), offset, stride);
    }

    void attach(const IndexBuffer& ebo) { glVertexArrayElementBuffer(id_, ebo.id()); }

    void config_attribute(AttribClass attrib_class, GLuint attrib_idx, GLint size, GLenum type,
                          GLuint relative_offset, GLuint binding_idx = 0) {
        glEnableVertexArrayAttrib(id_, attrib_idx);
        glVertexArrayAttribBinding(id_, attrib_idx, binding_idx);

        switch (attrib_class) {
            case AttribClass::Float:
                glVertexArrayAttribFormat(id_, attrib_idx, size, type, GL_FALSE, relative_offset);
                break;

            case AttribClass::Integer:
                glVertexArrayAttribIFormat(id_, attrib_idx, size, type, relative_offset);
                break;

            case AttribClass::Double:
                glVertexArrayAttribLFormat(id_, attrib_idx, size, type, relative_offset);
                break;

            default:
                Logger::critical_error(
                    "VertexArray",
                    std::format("invalid value passed as attrib_class to config_attribute ({})",
                                static_cast<uint8_t>(attrib_class)));
                break;
        }
    }

    // this isn't particularly pretty, but i wanna both keep attrib_class as first arg, and have it
    // be omittable for floats
    // just make sure to update this if config_attribute signature changes
    void config_attribute(GLuint attrib_idx, GLint size, GLenum type, GLuint relative_offset,
                          GLuint binding_idx = 0) {
        config_attribute(AttribClass::Float, attrib_idx, size, type, relative_offset, binding_idx);
    }

    // provided to allow specification of the 'normalized' argument
    // prefer config_attribute(AttribClass::Float, ...) when not using normalization
    void config_attribute_f(GLuint attrib_idx, GLint size, GLenum type, bool normalized,
                            GLuint relative_offset, GLuint binding_idx = 0) {
        glEnableVertexArrayAttrib(id_, attrib_idx);
        glVertexArrayAttribBinding(id_, attrib_idx, binding_idx);
        glVertexArrayAttribFormat(id_, attrib_idx, size, type, normalized ? GL_TRUE : GL_FALSE,
                                  relative_offset);
    }

    void bind() { glBindVertexArray(id_); }
    void unbind() { glBindVertexArray(0); }

    GLuint id() const { return id_; }

    DEFINE_VALIDATOR({ assert(id_ != 0 && "VertexArray with uninitialized or moved-from handle"); })
};
