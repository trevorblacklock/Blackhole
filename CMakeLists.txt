cmake_minimum_required(VERSION 3.28)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(blackhole)

include(cmake/CPM.cmake)

CPMAddPackage("gh:glfw/glfw#3.4")
CPMAddPackage("gh:g-truc/glm#1.0.1")
CPMAddPackage("gh:Dav1dde/glad#v0.1.36")

add_custom_target(copy_shaders ALL
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_BINARY_DIR}/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/src/shaders"
            "${CMAKE_BINARY_DIR}/shaders"
)

add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_BINARY_DIR}/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/src/assets"
            "${CMAKE_BINARY_DIR}/assets"
)

file(GLOB source CONFIGURE_DEPENDS "src/*.cpp")

add_executable(${PROJECT_NAME} ${source})

target_include_directories(${PROJECT_NAME} PRIVATE include/)

target_link_libraries(${PROJECT_NAME}
PUBLIC
  glad
  glfw
  glm
)

target_compile_definitions(
    ${PROJECT_NAME}
    PUBLIC
    SHADER_DIR="${CMAKE_BINARY_DIR}/shaders"
    ASSETS_DIR="${CMAKE_BINARY_DIR}/assets")
