file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(${APP_TARGET}
    ${APP_SOURCES}
)

target_include_directories(${APP_TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${APP_TARGET} PRIVATE
    ${ENGINE_TARGET}
)

apply_warnings(${APP_TARGET})
