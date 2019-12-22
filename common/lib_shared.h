#pragma once

#if defined(_MSC_VER) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#  define SHARED_DECL_EXPORT __declspec(dllexport)
#  define SHARED_DECL_IMPORT __declspec(dllimport)
#else
#  define SHARED_DECL_EXPORT __attribute__((visibility("default")))
#  define SHARED_DECL_IMPORT __attribute__((visibility("default")))
#endif
