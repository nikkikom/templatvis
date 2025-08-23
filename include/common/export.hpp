#pragma once

#if defined(_WIN32)
  #if defined(COMMON_BUILD)
    #define COMMON_API __declspec(dllexport)
  #else
    #define COMMON_API __declspec(dllimport)
  #endif
#else
  // GCC/Clang: по умолчанию всё hidden, экспорт явно
  #if defined(COMMON_BUILD)
    #define COMMON_API __attribute__((visibility("default")))
  #else
    #define COMMON_API
  #endif
#endif