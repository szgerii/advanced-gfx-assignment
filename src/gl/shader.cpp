#include "gl/shader.h"
#include "logger.h"

#include <format>
#include <fstream>
#include <string_view>

namespace {

constexpr std::string_view shader_stage_type_str(GLenum stage_type) {
    // clang-format off
    switch (stage_type) {
        case GL_COMPUTE_SHADER:         return "COMPUTE";
        case GL_VERTEX_SHADER:          return "VERTEX";
        case GL_TESS_CONTROL_SHADER:    return "TESSELLATION CONTROL";
        case GL_TESS_EVALUATION_SHADER: return "TESSELLATION EVALUATION";
        case GL_GEOMETRY_SHADER:        return "GEOMETRY";
        case GL_FRAGMENT_SHADER:        return "FRAGMENT";
        case GL_TASK_SHADER_NV:         return "TASK";
        case GL_MESH_SHADER_NV:         return "MESH";
        default:                        return "UNKNOWN OR INVALID SHADER STAGE TYPE";
    }
    // clang-format on
}

} // namespace

void Shader::compile(bool force_recreate) {
    // don't validate when called from ctor (or moved-from state)
    if (id_ != 0) {
        VALIDATE(*this);
    }

    if (force_recreate && id_ != 0) {
        glDeleteShader(id_);
        id_ = 0;
    }

    if (id_ == 0)
        id_ = glCreateShader(stage_type);

    if (id_ == 0)
        Logger::critical_error("Shader", "failed to create shader object");

    load_src();

    const char* src_data = src.data();
    GLint src_length     = static_cast<GLint>(src.length());
    glShaderSource(id_, 1, &src_data, &src_length);

    glCompileShader(id_);

    GLint result, info_log_length;
    glGetShaderiv(id_, GL_COMPILE_STATUS, &result);
    glGetShaderiv(id_, GL_INFO_LOG_LENGTH, &info_log_length);

    if (info_log_length > 0) {
        std::string info_log_buffer(static_cast<size_t>(info_log_length), '\0');
        glGetShaderInfoLog(id_, info_log_length, nullptr, info_log_buffer.data());

        if (result == GL_FALSE) {
            Logger::critical_error(
                "Shader",
                std::format(
                    "compilation of shader failed with the following info message from OpenGL:\n{}",
                    info_log_buffer));
        } else {
            Logger::warn("Shader", std::format("compilation of shader succeeded, but OpenGL raised "
                                               "the following info message during compilation:\n{}",
                                               info_log_buffer));
        }
    }

    if (result == GL_FALSE)
        Logger::critical_error("Shader", "compilation of shader failed without an error message "
                                         "queriable via glGetShaderInfoLog");

    VALIDATE(*this);
}

void Shader::load_src() {
    if (!src_path.has_value())
        return;

    auto fail_with_error = [&](std::string_view msg) {
        Logger::critical_error(
            "Shader",
            std::format(
                "{} [while trying to read shader source file at '{}' with stage_type type '{}']",
                msg, std::filesystem::absolute(*src_path).string(),
                shader_stage_type_str(stage_type)));
    };

    std::ifstream file{*src_path, std::ios::binary | std::ios::ate};
    if (file.fail())
        fail_with_error("failed to open file");

    const auto size = static_cast<std::streamsize>(file.tellg());
    if (size < 0)
        fail_with_error("failed to determine file size via tellg()");

    file.seekg(0, std::ios::beg);
    if (file.fail())
        fail_with_error("failed to seek beginning of file");

    src.resize(static_cast<size_t>(size));
    file.read(src.data(), size);

    if (file.fail())
        fail_with_error("failed to read contents of file");
}
