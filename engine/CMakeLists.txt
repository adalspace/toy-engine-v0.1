option(ENGINE_BUILD_SHARED "Build the Engine library as a shared library" ON)

set(SOURCES
  src/IO/parser.cpp
  src/IO/file_manager.cpp
  src/renderer/debug.cpp

  src/input/input.cpp

  src/scene/scene.cpp
  src/window/window.cpp

  src/components/batch.cpp
  src/renderer/mesh.cpp
  src/renderer/shader.cpp
  src/renderer/texture.cpp
  src/renderer/renderer.cpp

  src/renderer/wavefront.cpp

  src/renderer/core.cpp
)

if(ENGINE_BUILD_SHARED)
  add_library(${ENGINE_TARGET} SHARED ${SOURCES})
  if(WIN32)
    target_compile_definitions(${ENGINE_TARGET} PRIVATE ENGINE_BUILD_SHARED ENGINE_EXPORTS)
  else()
    target_compile_definitions(${ENGINE_TARGET} PRIVATE ENGINE_BUILD_SHARED)
  endif()
else()
  add_library(${ENGINE_TARGET} STATIC ${SOURCES})
  target_compile_definitions(${ENGINE_TARGET} PRIVATE ENGINE_STATIC)
endif()

set_target_properties(${ENGINE_TARGET} PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN YES
)

target_include_directories(${ENGINE_TARGET} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/contrib
)
target_link_libraries(${ENGINE_TARGET} PUBLIC
  SDL3::SDL3
  OpenGL::GL
  GLEW::GLEW
  glm::glm
  EnTT::EnTT
)

if (MSVC)
  target_compile_options(${ENGINE_TARGET} PRIVATE $<$<CONFIG:Debug>:/Zi>)
  target_link_options(${ENGINE_TARGET}    PRIVATE $<$<CONFIG:Debug>:/DEBUG:FULL>)
else()
  target_compile_options(${ENGINE_TARGET} PRIVATE $<$<CONFIG:Debug>:-ggdb>)
endif()

# Release flags
if (MSVC)
  # /O2: optimize speed, /GL: whole program opt (LTCG), /DNDEBUG: disable asserts
  target_compile_options(${ENGINE_TARGET} PRIVATE
    $<$<CONFIG:Release>:/O2>
    $<$<CONFIG:Release>:/DNDEBUG>
    $<$<CONFIG:RelWithDebInfo>:/O2>
  )
  # Link-time codegen & extra linker opts for smaller/faster binaries
  target_link_options(${ENGINE_TARGET} PRIVATE
    $<$<CONFIG:Release>:/LTCG /OPT:ICF /OPT:REF>
    $<$<CONFIG:RelWithDebInfo>:/LTCG /OPT:ICF /OPT:REF>
  )
else()
  # GCC/Clang
  # -O3 for max opts, -ffast-math optional but can be risky; we keep it OFF by default.
  option(CODINGGAME_USE_MARCH_NATIVE "Enable -march=native on Release for this machine" ON)
  target_compile_options(${ENGINE_TARGET} PRIVATE
    $<$<CONFIG:Release>:-O3>
    $<$<AND:$<CONFIG:Release>,$<BOOL:${CODINGGAME_USE_MARCH_NATIVE}>>:-march=native>
    $<$<CONFIG:Release>:-DNDEBUG>
    $<$<CONFIG:RelWithDebInfo>:-O3 -g>
  )
  # Linker: enable LTO when available; optionally strip symbols on non-Apple
  include(CheckIPOSupported)  # IPO == LTO in CMake terms
  check_ipo_supported(RESULT ipo_supported OUTPUT ipo_msg)
  if(ipo_supported)
    set_property(TARGET ${ENGINE_TARGET} PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
    set_property(TARGET ${ENGINE_TARGET} PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE)
  endif()

  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT APPLE)
    # -s strips symbols at link stage (use only for pure Release)
    target_link_options(${ENGINE_TARGET} PRIVATE $<$<CONFIG:Release>:-s>)
  endif()
endif()

