# OpenGL App

# Project Overview

Make sure to clone with submodules included: `git clone --recurse-submodules <url>`

If you've already cloned without submodules: `git submodule update --init --recursive`

Directory structure:
- *engine* is for general, framework-level stuff, base classes, helpers, architecture, etc.
- *app* is for the specific application, derived classes (implementations), etc.
- *shaders* is for shaders for either of the above targets (might be separated later)
- *vendor* is for build-from-source libraries

The *engine* is compiled and linked to *app* as a static library, including all vendor projects that are built from source (and exposing them as PUBLIC in CMake).

Entry point: `app/src/main.cpp`

# Dependencies

Dependencies marked as *source* are available under `vendor/`, dependencies marked as *FetchContent* are acquired by CMake's FetchContent module via Git automatically.

- [OpenGL 4.6](https://www.opengl.org/) (expected to be available on the host and locatable via `find_package(OpenGL)` in CMake)
- [glad2](https://github.com/dav1dde/glad) (source, generated via the official [Loader-Generator](https://gen.glad.sh/))
- [GLFW 3.4](https://github.com/glfw/glfw/tree/3.4) (FetchContent)
- [ImGui 1.92.8 (docking)](https://github.com/ocornut/imgui/tree/v1.92.8-docking) (source, submodule)
- [GLM 1.0.3](https://github.com/g-truc/glm/tree/1.0.3) (FetchContent)
- [stb](https://github.com/nothings/stb) (source, submodule)
- [debugbreak](https://github.com/scottt/debugbreak) (source, submodule)
- [ccache](https://ccache.dev/) (automatically enabled by CMake when available on the host)
