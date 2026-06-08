#pragma once

#include "gl/textures.h"
#include "vendor.h"

class Framebuffer {
    GLuint id_ = 0;

public:
    explicit Framebuffer() { glCreateFramebuffers(1, &id_); }

    ~Framebuffer() {
        if (id_ != 0)
            glDeleteFramebuffers(1, &id_);
    }

    Framebuffer(const Framebuffer&)            = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    Framebuffer(Framebuffer&& other) noexcept
        : id_{other.id_} {
        other.id_ = 0;
    }

    Framebuffer& operator=(Framebuffer&& other) noexcept {
        if (this == &other)
            return *this;

        if (id_ != 0)
            glDeleteFramebuffers(1, &id_);

        id_       = other.id_;
        other.id_ = 0;

        return *this;
    }

    void attach(const Texture& texture, GLuint attachment, GLint mipmap_level = 0) {
        glNamedFramebufferTexture(id_, attachment, texture.id(), mipmap_level);
    }

    void attach_color(const Texture& texture, GLuint attachment_idx = 0, GLint mipmap_level = 0) {
        attach(texture, GL_COLOR_ATTACHMENT0 + attachment_idx, mipmap_level);
    }

    void attach_depth(const Texture& texture, GLint mipmap_level = 0) {
        attach(texture, GL_DEPTH_ATTACHMENT, mipmap_level);
    }

    void attach_stencil(const Texture& texture, GLint mipmap_level = 0) {
        attach(texture, GL_STENCIL_ATTACHMENT, mipmap_level);
    }

    void attach_depth_stencil(const Texture& texture, GLint mipmap_level = 0) {
        attach(texture, GL_DEPTH_STENCIL_ATTACHMENT, mipmap_level);
    }

    bool complete() const {
        GLenum status = glCheckNamedFramebufferStatus(id_, GL_FRAMEBUFFER);
        return status == GL_FRAMEBUFFER_COMPLETE;
    }

    void bind() const { glBindFramebuffer(GL_FRAMEBUFFER, id_); }
    void unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

    GLuint id() const { return id_; }
};
