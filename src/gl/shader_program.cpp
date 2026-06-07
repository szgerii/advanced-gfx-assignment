#include "gl/shader_program.h"

#include <format>

#include "logger.h"

void ShaderProgram::link(bool force_recreate) {
    VALIDATE_ALL(stages);

    if (force_recreate && program_id_ != 0) {
        glDeleteProgram(program_id_);

        for (auto& stage : stages)
            stage.attached_ = false;
    }

    if (program_id_ == 0)
        program_id_ = glCreateProgram();

    if (program_id_ == 0)
        Logger::critical_error("Program", "failed to create program object");

    update_attachments();

    glLinkProgram(program_id_);

    GLint result = 0, info_log_length = 0;
    glGetProgramiv(program_id_, GL_LINK_STATUS, &result);
    glGetProgramiv(program_id_, GL_INFO_LOG_LENGTH, &info_log_length);

    if (info_log_length > 0) {
        std::string info_log_buffer(static_cast<size_t>(info_log_length), '\0');
        glGetProgramInfoLog(program_id_, info_log_length, nullptr, info_log_buffer.data());

        if (result == GL_FALSE) {
            Logger::critical_error(
                "Program",
                std::format(
                    "linking of program failed with the following info message from OpenGL:\n{}",
                    info_log_buffer));
        } else {
            Logger::warn("Program",
                         std::format("linking of program succeeded, but OpenGL raised "
                                     "the following info message during compilation:\n{}",
                                     info_log_buffer));
        }
    }

    if (result == GL_FALSE)
        Logger::critical_error("Program", "linking of program failed without an error message "
                                          "queriable via glGetProgramInfoLog");

    VALIDATE(*this);

    // shader objects could be deleted here for deferred deletion (if the program "owns" them)
    // however, since Shader was made a RAII struct instead, to allow more flexible and
    // explicit lifetime management for shader resources
}

void ShaderProgram::build(bool force_recreate_program, bool force_recreate_shaders) {
    VALIDATE_ALL(stages);

    for (auto& stage : stages)
        stage.shader->compile(force_recreate_shaders);

    link(force_recreate_program);

    VALIDATE(*this);
}

GLint ShaderProgram::uniform_location(std::string_view name) const {
    VALIDATE(*this);

    if (auto it = uniform_cache_.find(name); it != uniform_cache_.end())
        return it->second;

    std::string name_str{name};
    GLint loc = glGetUniformLocation(program_id_, name_str.c_str());

    if (loc == -1) {
        Logger::error("Shader", std::format("Trying to access location of uniform with name '{}', "
                                            "which was not found in the targeted shader program",
                                            name));
    }

    uniform_cache_.emplace(std::move(name_str), loc);

    return loc;
}

// TODO: DRY above and below fns

void ShaderProgram::load(std::string_view name) const {
    if (uniform_cache_.contains(name))
        return;

    std::string name_str{name};
    GLint loc = glGetUniformLocation(program_id_, name_str.c_str());

    if (loc == -1) {
        Logger::error("Shader", std::format("Trying to access location of uniform with name '{}', "
                                            "which was not found in the targeted shader program",
                                            name));
    }

    uniform_cache_.emplace(std::move(name_str), loc);
}

void ShaderProgram::load(const std::vector<std::string_view>& names) const {
    for (std::string_view name : names)
        load(name);
}

void ShaderProgram::update_attachments() {
    VALIDATE_ALL(stages);

    for (auto& stage : stages) {
        if (stage.active && !stage.attached_) {
            glAttachShader(program_id_, stage.shader->id());
            stage.attached_ = true;
        }

        if (!stage.active && stage.attached_) {
            glDetachShader(program_id_, stage.shader->id());
            stage.attached_ = false;
        }
    }
}
