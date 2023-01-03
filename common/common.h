#pragma once

#include "concepts.h"
#include "point.h"
#include "string.h"
#include "utility.h"

#if defined(NDEBUG)
#if defined(__assume) || defined(_MSC_VER)
#define AOC_ASSERT_HELPER(condition, message) __assume(condition)
#elif defined(__has_builtin)
#if __has_builtin(__builtin_assume)
#define AOC_ASSERT_HELPER(condition, message) __builtin_assume(condition)
#else // __builtin_assume not available
#define AOC_ASSERT_HELPER(condition, message) ((void)0)
#endif // __has_builtin(__builtin_assume)
#else  // __assume not available
#define AOC_ASSERT_HELPER(condition, message) ((void)0)
#endif // __assume  || _MSC_VER
#else
#include <cassert>
#define AOC_ASSERT_HELPER(condition, message) assert((condition) && (message))
#endif // NDEBUG

#define AOC_ASSERT(condition, message) AOC_ASSERT_HELPER((condition), (message))

static bool g_success = true;

#define AOC_EXPECT_RESULT(expected, actual)                                    \
  {                                                                            \
    constexpr auto expected_v = (expected);                                    \
    const auto actual_v = (actual);                                            \
    if (expected_v != actual_v) {                                              \
      std::cout << "Wrong result! Expected " << expected_v << ", got "         \
                << actual_v << std::endl;                                      \
      g_success = false;                                                       \
    }                                                                          \
  }

#define AOC_RETURN_CHECK_RESULT()                                              \
  if (g_success) {                                                             \
    return 0;                                                                  \
  } else {                                                                     \
    return -1;                                                                 \
  }
