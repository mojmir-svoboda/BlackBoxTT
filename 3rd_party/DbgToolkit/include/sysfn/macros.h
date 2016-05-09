#pragma once

#define CACHE_LINE 64

#if defined _MSC_VER
#   define CACHE_ALIGN __declspec(align(CACHE_LINE))
#   define ALIGN(n) __declspec(align(n))
#else
#  define CACHE_ALIGN __attribute__((aligned(CACHE_LINE)))
#  define ALIGN(n) __attribute__((aligned(n)))
#endif

