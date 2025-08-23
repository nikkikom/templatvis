#pragma once
#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(DEMO_BUILD)
#define DEMO_EXPORT __declspec(dllexport)
#else
#define DEMO_EXPORT __declspec(dllimport)
#endif
#define DEMO_HIDDEN
#else
#if defined(DEMO_BUILD)
#define DEMO_EXPORT __attribute__((visibility("default")))
#else
#define DEMO_EXPORT
#endif
#define DEMO_HIDDEN __attribute__((visibility("hidden")))
#endif
