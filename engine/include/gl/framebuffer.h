#pragma once

#include "gl/textures.h"
#include "vendor.h"

#include <format>

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

    // use for manual setup of FBO draw buffers
    void set_draw_buffers(std::span<const GLenum> buffers) {
#ifndef NDEBUG
        assert_valid_draw_buffer_count(num_cast<GLint>(buffers.size()));
#endif

        glNamedFramebufferDrawBuffers(id_, num_cast<GLsizei>(buffers.size()), buffers.data());
    }

    // use for automatic setup of FBO draw buffers, color attachments [0, n)
    void set_draw_buffers(GLsizei n) {
#ifndef NDEBUG
        assert_valid_draw_buffer_count(num_cast<GLint>(n));
#endif

        static std::vector<GLenum> buffers(max_draw_buffers());

        for (int i = 0; i < n; i++)
            buffers[i] = GL_COLOR_ATTACHMENT0 + i;

        glNamedFramebufferDrawBuffers(id_, n, buffers.data());
    }

    bool complete() const {
        GLenum status = glCheckNamedFramebufferStatus(id_, GL_FRAMEBUFFER);
        return status == GL_FRAMEBUFFER_COMPLETE;
    }

    void bind() const { glBindFramebuffer(GL_FRAMEBUFFER, id_); }
    void unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

    GLuint id() const { return id_; }

    GLint max_draw_buffers() const {
        static GLint max_draw_buffers = -1;
        if (max_draw_buffers == -1)
            glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &max_draw_buffers);

        return max_draw_buffers;
    }

private:
    void assert_valid_draw_buffer_count(GLint n) const {
        if (max_draw_buffers() == 0) {
            Logger::critical_error("Framebuffer",
                                   "Trying to use multiple draw buffers on an FBO, but the "
                                   "current context doesn't support it");
        }

        if (n > max_draw_buffers()) {
            Logger::critical_error("Framebuffer",
                                   std::format("Trying to use {} draw buffers on an FBO, but "
                                               "the current context only supports a maximum of {}.",
                                               n, max_draw_buffers()));
        }
    }
};
