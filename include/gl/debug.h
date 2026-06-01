#pragma once

#include "vendor.h"

bool is_debugger_present();

void GLAPIENTRY ogl_debug_msg_callback(GLenum src, GLenum type, GLuint id, GLenum severity,
                                       GLsizei length, const GLchar* msg, const void* user_param);
