#ifndef AOC_UTILITY_H
#define AOC_UTILITY_H

// TODO: Everything left here still needs a proper home:
// the insertion helpers belong with the container vocabulary,
// flush and return_incomplete with the test harness in common.h

#include "compiler.h"
#include "concepts.h"

#ifndef AOC_MODULE_SUPPORT
#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <print>
#include <string>
#include <thread>
#endif
#endif

AOC_EXPORT_NAMESPACE(aoc) {

// How many threads the parallel solutions should fan out over.
//
// The MSVC debug CRT threads every allocation onto one global list
// behind a single lock,
// so past a handful of threads the contention outweighs the parallelism:
// the same total alloc work measures 1.2s on one thread against 12.3s on 32.
// glibc gives each thread its own arena and never shows this.
//
// 4 is where the measured solutions balance out.
#if defined(AOC_COMPILER_MSVC) && defined(_DEBUG)
#ifndef AOC_DEBUG_THREAD_CAP
#define AOC_DEBUG_THREAD_CAP 4
#endif
#endif

inline unsigned num_worker_threads() {
  const auto available = std::max(1u, std::thread::hardware_concurrency());
#ifdef AOC_DEBUG_THREAD_CAP
  return std::min(available, unsigned{AOC_DEBUG_THREAD_CAP});
#else
  return available;
#endif
}

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
