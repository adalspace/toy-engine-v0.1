#pragma once

#if defined(_WIN32) && defined(ENGINE_BUILD_SHARED)
  #define ENGINE_API __declspec(dllexport)
#elif defined(_WIN32)
  #define ENGINE_API __declspec(dllimport)
#else
  #define ENGINE_API __attribute__((visibility("default")))
#endif