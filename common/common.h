#ifndef AOC_COMMON_H
#define AOC_COMMON_H

// These two files need to be included here because the macros they define
// are not exported by the module
#define AOC_INCLUDE_FROM_COMMON
#include "assert.h"
#include "compiler.h"

#if !defined(AOC_DISABLE_MODULES) &&                                           \
    defined(AOC_MODULE_SUPPORT) &&                                             \
    !defined(__INTELLISENSE__)

import aoc;

#else

// Include same headers as the module
#include "algorithm.h"
#include "assert.h"
#include "compiler.h"
#include "concepts.h"
#include "flat.h"
#include "grid.h"
#include "math.h"
#include "point.h"
#include "range_to.h"
#include "ranges.h"
#include "ratio.h"
#include "static_vector.h"
#include "string.h"
#include "timer.h"
#include "utility.h"

// We're using std::print in this file
#include <print>

#endif

using point = aoc::point_type<int>;

static bool g_success = true;

#define AOC_EXPECT_RESULT(expected, actual)                                    \
  {                                                                            \
    constexpr auto expected_v = (expected);                                    \
    aoc::timer timer_;                                                         \
    timer_.start();                                                            \
    const auto actual_v = (actual);                                            \
    timer_.stop();                                                             \
    if (expected_v != actual_v) {                                              \
      std::println("  FAIL {} ms: expected {}, got {}", timer_.milliseconds(), \
                   expected_v, actual_v);                                      \
      g_success = false;                                                       \
    } else {                                                                   \
      std::println("  OK {} ms", timer_.milliseconds());                       \
    }                                                                          \
  }

#define AOC_RETURN_CHECK_RESULT()                                              \
  if (g_success) {                                                             \
    return 0;                                                                  \
  } else {                                                                     \
    return -1;                                                                 \
  }

#endif // AOC_COMMON_H
