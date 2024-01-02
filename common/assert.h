#pragma once

#define AOC_NOT_CONSTEXPR(statement)                                           \
  if (!std::is_constant_evaluated()) {                                         \
    statement;                                                                 \
  }

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

#define AOC_ASSERT(condition, message)                                         \
  AOC_NOT_CONSTEXPR(AOC_ASSERT_HELPER((condition), (message)))                 \
  else if (!(condition)) {                                                     \
    throw 0;                                                                   \
  }
