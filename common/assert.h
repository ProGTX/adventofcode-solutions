#ifndef AOC_ASSERT_H
#define AOC_ASSERT_H

#include "compiler.h"

#if defined(NDEBUG)
#if defined(__assume) || defined(_MSC_VER)
#define AOC_ASSERT_HELPER(condition, message) __assume(condition)
#elif defined(__has_builtin)
#if __has_builtin(__builtin_assume)
#ifdef AOC_COMPILER_CLANG
// Most of our conditions call functions,
// which Clang treats as having potential side effects,
// so it drops the assumption and warns about it.
// The dropped assumption is fine, the warning is just noise.
// Push/pop around the expansion doesn't work here,
// the warning also fires from template instantiations
// that happen long after the macro has been expanded.
#pragma clang diagnostic ignored "-Wassume"
#endif // AOC_COMPILER_CLANG
#define AOC_ASSERT_HELPER(condition, message) __builtin_assume(condition)
#else // __builtin_assume not available
#define AOC_ASSERT_HELPER(condition, message) ((void)0)
#endif // __has_builtin(__builtin_assume)
#else  // __assume not available
#define AOC_ASSERT_HELPER(condition, message) ((void)0)
#endif // __assume  || _MSC_VER
#else
#ifdef AOC_INCLUDE_FROM_COMMON
#include <cassert>
#endif
#define AOC_ASSERT_HELPER(condition, message) assert((condition) && (message))
#endif // NDEBUG

#define AOC_ASSERT(condition, message)                                         \
  AOC_NOT_CONSTEXPR(AOC_ASSERT_HELPER((condition), (message)))                 \
  else if (!(condition)) {                                                     \
    throw 0;                                                                   \
  }

// In theory we could use std::unreachable,
// but it seems to conflict with modules
#define AOC_UNREACHABLE(message) AOC_ASSERT(false, message)

#endif // AOC_ASSERT_H
