#pragma once

// For static libs, ENGINE_API should be empty.
#if defined(_WIN32)
  #if defined(ENGINE_BUILD_SHARED)
    #if defined(ENGINE_EXPORTS)
      #define ENGINE_API __declspec(dllexport)
    #else
      #define ENGINE_API __declspec(dllimport)
    #endif
  #else
    #define ENGINE_API
  #endif
#else
  #if defined(ENGINE_BUILD_SHARED)
    #define ENGINE_API __attribute__((visibility("default")))
  #else
    #define ENGINE_API
  #endif
#endif