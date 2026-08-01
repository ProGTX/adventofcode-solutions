#ifndef AOC_UTILITY_H
#define AOC_UTILITY_H

// TODO: Everything left here still needs a proper home:
// the insertion helpers belong with the container vocabulary,
// flush and return_incomplete with the test harness in common.h

#include "compiler.h"
#include "concepts.h"

#ifndef AOC_MODULE_SUPPORT
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <print>
#include <string>
#endif

AOC_EXPORT_NAMESPACE(aoc) {

template <class output_t>
constexpr auto inserter_it(output_t& elems) {
  if constexpr (back_insertable<output_t>) {
    return std::back_inserter(elems);
  } else if constexpr (insertable<output_t>) {
    return std::inserter(elems, std::end(elems));
  } else if constexpr (specialization_of<output_t, std::optional>) {
    return std::addressof(elems);
  } else {
    return std::begin(elems);
  }
}

template <class output_t>
constexpr auto insertion_end_it(output_t& elems) {
  if constexpr (specialization_of<output_t, std::optional>) {
    return std::addressof(elems) + 1;
  } else if constexpr (const auto N = static_size<output_t>();
                       N != std::string::npos) {
    return std::begin(elems) + N;
  } else {
    return std::unreachable_sentinel;
  }
}

/// Custom flush function so that users of the aoc module
/// don't have to include iostream
void flush() { std::cout << std::flush; }

void return_incomplete() {
  std::println("  This test is incomplete, skipping");
  flush();
  std::exit(AOC_SKIP_RETURN_CODE);
}

} // AOC_EXPORT_NAMESPACE(aoc)

#endif // AOC_UTILITY_H
