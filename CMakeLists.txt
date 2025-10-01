cmake_minimum_required(VERSION 3.16)
project(CodingGame LANGUAGES C CXX)

# --- deps via vcpkg ---
# (vcpkg installs decide static vs shared; no "SDL3-shared" component needed)
find_package(SDL3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

# --- exe ---
add_executable(CodingGame
  src/prelude.cpp
  src/file_manager.cpp
  src/shader.cpp
  src/block.cpp
  src/vertex.cpp
  src/texture.cpp
  src/model.cpp
  src/main.cpp
)

set_property(TARGET CodingGame PROPERTY CXX_STANDARD 17)
set_property(TARGET CodingGame PROPERTY CXX_STANDARD_REQUIRED ON)

file(COPY ${CMAKE_SOURCE_DIR}/src/shaders DESTINATION ${CMAKE_BINARY_DIR}/)

target_include_directories(CodingGame PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/contrib
)

target_link_libraries(CodingGame PRIVATE
  glm::glm
  OpenGL::GL
  SDL3::SDL3           # vcpkg’s SDL3 target
  GLEW::GLEW
)

# Debug flags per toolchain
if (MSVC)
  target_compile_options(CodingGame PRIVATE $<$<CONFIG:Debug>:/Zi>)
  target_link_options(CodingGame PRIVATE $<$<CONFIG:Debug>:/DEBUG:FULL>)
else()
  target_compile_options(CodingGame PRIVATE $<$<CONFIG:Debug>:-ggdb>)
endif()

# --- copy runtime DLLs next to the exe on Windows ---
# (CMake 3.21+)
if (WIN32)
  add_custom_command(TARGET CodingGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_RUNTIME_DLLS:CodingGame> $<TARGET_FILE_DIR:CodingGame>
    COMMAND_EXPAND_LISTS)
endif()
