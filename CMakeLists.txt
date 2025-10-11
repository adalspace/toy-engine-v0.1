cmake_minimum_required(VERSION 3.16)
project(CodingGame LANGUAGES C CXX)

# ---------- Build-type defaults (only affects single-config generators like Ninja/Make) ----------
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

# ---------- Dependencies ----------
if (UNIX)
  include(FetchContent)
  FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        bf71a834948186f4097caa076cd2663c69a10e1e # refs/tags/1.0.1
  )
  FetchContent_MakeAvailable(glm)

  FetchContent_Declare(
    EnTT
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG        d4014c74dc3793aba95ae354d6e23a026c2796db
  )
  FetchContent_MakeAvailable(EnTT)

  find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
  find_package(OpenGL REQUIRED)
  find_package(GLEW REQUIRED)
elseif (MSVC) # vcpkg
  find_package(SDL3 CONFIG REQUIRED)
  find_package(OpenGL REQUIRED)
  find_package(GLEW CONFIG REQUIRED)
  find_package(glm CONFIG REQUIRED)
  find_package(EnTT CONFIG REQUIRED)
endif()

add_executable(CodingGame
  src/IO/parser.cpp
  src/IO/file_manager.cpp

  src/window/window.cpp

  src/renderer/debug.cpp
  src/renderer/basics.cpp
  src/renderer/mesh.cpp
  src/renderer/shader.cpp
  src/renderer/texture.cpp
  src/renderer/wavefront.cpp
  src/renderer/engine.cpp
  src/renderer/renderer.cpp

  src/main.cpp
)

set_property(TARGET CodingGame PROPERTY CXX_STANDARD 17)
set_property(TARGET CodingGame PROPERTY CXX_STANDARD_REQUIRED ON)

file(COPY src/shaders DESTINATION ${CMAKE_BINARY_DIR}/)

target_include_directories(CodingGame PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/contrib
)
target_link_libraries(CodingGame PRIVATE
  SDL3::SDL3
  OpenGL::GL
  GLEW::GLEW
  glm::glm
  EnTT::EnTT
)

# ---------- Visibility (helps optimizer & smaller binaries on Release) ----------
# Only affects non-Windows compilers
set_target_properties(CodingGame PROPERTIES
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN YES
)

# ---------- Per-config flags ----------
# Debug flags (your original intent, kept)
if (MSVC)
  target_compile_options(CodingGame PRIVATE $<$<CONFIG:Debug>:/Zi>)
  target_link_options(CodingGame    PRIVATE $<$<CONFIG:Debug>:/DEBUG:FULL>)
else()
  target_compile_options(CodingGame PRIVATE $<$<CONFIG:Debug>:-ggdb>)
endif()

# Release flags
if (MSVC)
  # /O2: optimize speed, /GL: whole program opt (LTCG), /DNDEBUG: disable asserts
  target_compile_options(CodingGame PRIVATE
    $<$<CONFIG:Release>:/O2>
    $<$<CONFIG:Release>:/DNDEBUG>
    $<$<CONFIG:RelWithDebInfo>:/O2>
  )
  # Link-time codegen & extra linker opts for smaller/faster binaries
  target_link_options(CodingGame PRIVATE
    $<$<CONFIG:Release>:/LTCG /OPT:ICF /OPT:REF>
    $<$<CONFIG:RelWithDebInfo>:/LTCG /OPT:ICF /OPT:REF>
  )
else()
  # GCC/Clang
  # -O3 for max opts, -ffast-math optional but can be risky; we keep it OFF by default.
  option(CODINGGAME_USE_MARCH_NATIVE "Enable -march=native on Release for this machine" ON)
  target_compile_options(CodingGame PRIVATE
    $<$<CONFIG:Release>:-O3>
    $<$<AND:$<CONFIG:Release>,$<BOOL:${CODINGGAME_USE_MARCH_NATIVE}>>:-march=native>
    $<$<CONFIG:Release>:-DNDEBUG>
    $<$<CONFIG:RelWithDebInfo>:-O3 -g>
  )
  # Linker: enable LTO when available; optionally strip symbols on non-Apple
  include(CheckIPOSupported)  # IPO == LTO in CMake terms
  check_ipo_supported(RESULT ipo_supported OUTPUT ipo_msg)
  if(ipo_supported)
    set_property(TARGET CodingGame PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
    set_property(TARGET CodingGame PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE)
  endif()

  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT APPLE)
    # -s strips symbols at link stage (use only for pure Release)
    target_link_options(CodingGame PRIVATE $<$<CONFIG:Release>:-s>)
  endif()
endif()

# ---------- Windows: copy runtime DLLs ----------
if (WIN32)
  add_custom_command(TARGET CodingGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_RUNTIME_DLLS:CodingGame> $<TARGET_FILE_DIR:CodingGame>
    COMMAND_EXPAND_LISTS)
endif()