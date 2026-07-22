cmake_minimum_required(VERSION 3.16)
project(CodingGame LANGUAGES C CXX)

set(ENGINE_TARGET engine)

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
    GIT_TAG        a532f5b1cf27d6a3c099437e6959cf7e398a0a67 # refs/tags/1.0.2
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
  add_subdirectory(vendored/SDL3)
  add_subdirectory(vendored/glm)
  add_subdirectory(vendored/EnTT)
  find_package(GLEW CONFIG REQUIRED)
  find_package(OpenGL REQUIRED)
  # find_package(glm CONFIG REQUIRED)
  # find_package(EnTT CONFIG REQUIRED)
endif()

add_subdirectory(engine)
add_subdirectory(sandbox)
