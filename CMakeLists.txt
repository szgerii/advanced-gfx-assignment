cmake_minimum_required(VERSION 3.14)
project(AdvancedGraphicsAssignment LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_SHARED_LIBS OFF)

set(APP_TARGET app)
set(ENGINE_TARGET engine)

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

# other vendor stuff

set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/")
set(GLAD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/")
set(DEBUGBREAK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/debugbreak/")
set(STB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb/")

file(GLOB IMGUI_SOURCES_BASE CONFIGURE_DEPENDS "${IMGUI_DIR}/*.cpp")
set(IMGUI_ALL_SOURCES
    ${IMGUI_SOURCES_BASE}
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
)
set_source_files_properties(${IMGUI_ALL_SOURCES} PROPERTIES COMPILE_OPTIONS "-w")

# ==============================
#   Engine Target (static lib)
# ==============================

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS "engine/src/*.cpp")

add_library(${ENGINE_TARGET} STATIC
    ${ENGINE_SOURCES}

    ${GLAD_DIR}/src/gl.c
    ${IMGUI_ALL_SOURCES}
)

target_include_directories(${ENGINE_TARGET} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/include
)

target_include_directories(${ENGINE_TARGET} SYSTEM PUBLIC
    ${GLAD_DIR}/include
    ${GLAD_DIR}/include/glad
    ${GLAD_DIR}/include/KHR

    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${IMGUI_DIR}/misc/cpp

    ${glm_SOURCE_DIR} # to manually mark glm includes as SYSTEM 
    ${DEBUGBREAK_DIR}
    ${STB_DIR}
)

target_link_libraries(${ENGINE_TARGET} PUBLIC
    glfw
    OpenGL::GL
    glm::glm
)

# ============
#   App Exec
# ============

file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS "app/src/*.cpp")

add_executable(${APP_TARGET}
    ${APP_SOURCES}
)

target_include_directories(${APP_TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/app/include
)

target_link_libraries(${APP_TARGET} PRIVATE
    ${ENGINE_TARGET}
)

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

apply_warnings(${ENGINE_TARGET})
apply_warnings(${APP_TARGET})

set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/engine/src/utils/obj_parser.cpp" PROPERTIES COMPILE_OPTIONS "-w")

# this may need to be configured further if app uses large asset files
add_custom_command(
    TARGET ${APP_TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${APP_TARGET}>/assets"
)
