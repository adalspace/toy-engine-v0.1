set(SANDBOX_TARGET sandbox)

add_executable(${SANDBOX_TARGET} src/main.cpp)

set_target_properties(${SANDBOX_TARGET} PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
)

target_link_libraries(${SANDBOX_TARGET} PRIVATE ${ENGINE_TARGET})
