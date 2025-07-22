#ifndef AOC_COMMON_H
#define AOC_COMMON_H

// These two files need to be included here because the macros they define
// are not exported by the module
#define AOC_INCLUDE_FROM_COMMON
#include "assert.h"
#include "compiler.h"

#if !defined(AOC_DISABLE_MODULES) && defined(AOC_MODULE_SUPPORT) &&            \
    !defined(__INTELLISENSE__)

import aoc;

#else

// Include same headers as the module
#include "algorithm.h"
#include "assert.h"
#include "compiler.h"
#include "concepts.h"
#include "flat.h"
#include "graph.h"
#include "grid.h"
#include "math.h"
#include "point.h"
#include "print.h"
#include "range_to.h"
#include "ranges.h"
#include "ratio.h"
#include "static_vector.h"
#include "string.h"
#include "timer.h"
#include "utility.h"

// We're using std::cout and std::end in this file
#include <iostream>

#endif

using point = aoc::point_type<int>;

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

#endif // AOC_COMMON_H
