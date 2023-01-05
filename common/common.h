#pragma once

#include "concepts.h"
#include "point.h"
#include "string.h"
#include "utility.h"

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
