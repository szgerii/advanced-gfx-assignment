cmake_minimum_required(VERSION 3.14)
project(AdvancedGraphicsAssignment LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_SHARED_LIBS OFF)

set(ENGINE_TARGET engine)
set(ENGINE_DIRECTORY "${CMAKE_SOURCE_DIR}/engine")
set(APP_TARGET app)
set(APP_DIRECTORY "${CMAKE_SOURCE_DIR}/app")

option(USE_CCACHE "Use ccache for compiler caching, if it's installed locally" ON)

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
# source: https://github.com/g-truc/glm#cmake-using-fetchcontent

set(GLM_BUILD_TESTS OFF)

FetchContent_Declare(
	glm
	GIT_REPOSITORY https://github.com/g-truc/glm.git
	GIT_TAG 	   1.0.3 # latest as of development
)
FetchContent_MakeAvailable(glm)

# ogl

find_package(OpenGL REQUIRED COMPONENTS OpenGL)

# ccache

if (USE_CCACHE)
    find_program(CCACHE_EXEC ccache)
    if (CCACHE_EXEC)
        set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXEC}")
        set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_EXEC}")

        message(STATUS "ccache found and enabled for compiler caching")
    else()
        message(STATUS "ccache not found, compiler caching won't be possible")
    endif()
else()
    message(STATUS "ccache usage is disabled")
endif()

# other vendor stuff (used by engine's CMakeLists.txt)

set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/vendor/imgui/")
set(GLAD_DIR "${CMAKE_SOURCE_DIR}/vendor/glad/")
set(DEBUGBREAK_DIR "${CMAKE_SOURCE_DIR}/vendor/debugbreak/")
set(STB_DIR "${CMAKE_SOURCE_DIR}/vendor/stb/")

function(apply_warnings target_name)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target_name} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wsign-conversion
            -Wsign-compare
        )
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        target_compile_options(${target_name} PRIVATE
            /W4
            /permissive-
        )
    endif()
endfunction()

add_subdirectory(${ENGINE_DIRECTORY})
add_subdirectory(${APP_DIRECTORY})
