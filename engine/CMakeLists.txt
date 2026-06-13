file(GLOB IMGUI_SOURCES_BASE CONFIGURE_DEPENDS "${IMGUI_DIR}/*.cpp")
set(IMGUI_ALL_SOURCES
    ${IMGUI_SOURCES_BASE}
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
)
set_source_files_properties(${IMGUI_ALL_SOURCES} PROPERTIES COMPILE_OPTIONS "-w")

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(${ENGINE_TARGET} STATIC
    ${ENGINE_SOURCES}

    ${GLAD_DIR}/src/gl.c
    ${IMGUI_ALL_SOURCES}
)

target_compile_definitions(${ENGINE_TARGET} PUBLIC
    SRC_ROOT="${CMAKE_SOURCE_DIR}"
)

target_include_directories(${ENGINE_TARGET} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
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

apply_warnings(${ENGINE_TARGET})
set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/src/utils/obj_parser.cpp" PROPERTIES COMPILE_OPTIONS "-w")
