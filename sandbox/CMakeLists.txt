set(SANDBOX_TARGET sandbox)
set(MODEL_TARGET model)

add_executable(${SANDBOX_TARGET} src/main.cpp)
add_executable(${MODEL_TARGET} src/model.cpp)

set_target_properties(${SANDBOX_TARGET} PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
)

set_target_properties(${MODEL_TARGET} PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
)

target_link_libraries(${SANDBOX_TARGET} PRIVATE ${ENGINE_TARGET})
target_link_libraries(${MODEL_TARGET} PRIVATE ${ENGINE_TARGET})

# --- Copy engine.dll and all dependent DLLs next to sandbox.exe ---
if (WIN32)
    add_custom_command(TARGET ${SANDBOX_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:${SANDBOX_TARGET}> $<TARGET_FILE_DIR:${SANDBOX_TARGET}>
        COMMAND_EXPAND_LISTS
    )

    add_custom_command(TARGET ${MODEL_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:${MODEL_TARGET}> $<TARGET_FILE_DIR:${MODEL_TARGET}>
        COMMAND_EXPAND_LISTS
    )
endif()
