#pragma once
#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(THROWERS_BUILD)
#define THROWERS_EXPORT __declspec(dllexport)
#else
#define THROWERS_EXPORT __declspec(dllimport)
#endif
#define THROWERS_HIDDEN
#else
#if defined(THROWERS_BUILD)
#define THROWERS_EXPORT __attribute__((visibility("default")))
#else
#define THROWERS_EXPORT
#endif
#define THROWERS_HIDDEN __attribute__((visibility("hidden")))
#endif
