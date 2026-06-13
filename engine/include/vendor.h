// CENTRAL POINT FOR GRAPHICS VENDOR INCLUDES
// because inclusion order can be sensitive for glad/glfw, and they behaviour-modifying defines can
// easily get scattered across the codebase, inclusion should only be done via this header

#pragma once

#include <glad/gl.h>

#include <GLFW/glfw3.h>

#ifdef __cplusplus // glm imports <cmath>, breaking C compilation
    #include <glm/glm.hpp>
    #include <glm/gtc/type_ptr.hpp>
#endif
