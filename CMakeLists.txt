cmake_minimum_required(VERSION 3.14)
project(AdvancedGraphicsAssignment LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_SHARED_LIBS OFF)

set(MAIN_EXEC_TARGET assignment)

include(FetchContent)

# GLFW
# source: https://www.glfw.org/docs/3.4/build_guide.html#build_link_cmake_source

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4 # latest as of development
)
FetchContent_MakeAvailable(glfw)

# glm

set(GLM_BUILD_TESTS OFF)

FetchContent_Declare(
	glm
	GIT_REPOSITORY https://github.com/g-truc/glm.git
	GIT_TAG 	   1.0.3 # latest as of development
)
FetchContent_MakeAvailable(glm)

# ogl

find_package(OpenGL REQUIRED)

# other vendor stuff

set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/")
set(GLAD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/")
set(DEBUGBREAK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/debugbreak/")

file(GLOB IMGUI_SOURCES_BASE CONFIGURE_DEPENDS "${IMGUI_DIR}/*.cpp")
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(${MAIN_EXEC_TARGET}
    ${PROJECT_SOURCES}

    ${GLAD_DIR}/src/gl.c

    ${IMGUI_SOURCES_BASE}
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
)

target_include_directories(${MAIN_EXEC_TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    
    ${GLAD_DIR}/include
    ${GLAD_DIR}/include/glad
    ${GLAD_DIR}/include/KHR

    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${IMGUI_DIR}/misc/cpp

    ${DEBUGBREAK_DIR}
)

target_link_libraries(${MAIN_EXEC_TARGET} PRIVATE
    glfw
    OpenGL::GL
    glm::glm
)
