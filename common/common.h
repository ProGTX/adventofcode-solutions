#ifndef AOC_COMMON_H
#define AOC_COMMON_H

// These two files need to be included here because the macros they define
// are not exported by the module
#define AOC_INCLUDE_FROM_COMMON
#include "assert.h"
#include "compiler.h"
// Not part of the module: it needs the CRT headers directly,
// and every test wants it regardless of how the rest of aoc is pulled in
#include "crt_report.h"

#if !defined(AOC_DISABLE_MODULES) &&                                           \
    defined(AOC_MODULE_SUPPORT) &&                                             \
    !defined(__INTELLISENSE__)
// Pure module path
import aoc;

#ifdef AOC_IMPORT_STD
import std;
#else
#include <ranges>
#endif // AOC_IMPORT_STD

#else
// Header path

#ifdef AOC_IMPORT_STD
// MSVC only tolerates textual std headers in a TU
// that also imports std when they are parsed *before* the import.
// unordered_dense is a plain header
// that includes a good part of the standard library,
// so it has to be hoisted here out of hash.h.
#include <ankerl/unordered_dense.h>
// The aoc headers below skip their own std includes, so this has to come first
import std;

#endif // AOC_IMPORT_STD

// Include same headers as the module
#include "algorithm.h"
#include "assert.h"
#include "bitmap_set.h"
#include "combinations.h"
#include "compiler.h"
#include "concepts.h"
#include "dijkstra.h"
#include "flat.h"
#include "functional.h"
#include "grid.h"
#include "hash.h"
#include "math.h"
#include "md5.h"
#include "parse.h"
#include "point.h"
#include "range_to.h"
#include "ranges.h"
#include "ratio.h"
#include "static_vector.h"
#include "string.h"
#include "timer.h"
#include "utility.h"

// We're using std::print and ranges in this file
#ifndef AOC_IMPORT_STD
#include <print>
#include <ranges>
#endif // AOC_IMPORT_STD

#endif // modules

namespace stdr = std::ranges;
namespace stdv = std::views;
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
    aoc::flush();                                                              \
  }

#define AOC_RETURN_CHECK_RESULT()                                              \
  if (g_success) {                                                             \
    return 0;                                                                  \
  } else {                                                                     \
    return -1;                                                                 \
  }

#endif // AOC_COMMON_H
