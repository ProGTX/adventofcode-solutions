// https://adventofcode.com/2015/day/4

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <array>
#include <atomic>
#include <charconv>
#include <limits>
#include <print>
#include <span>
#include <thread>
#include <vector>
#endif

/// How many numbers are hashed side by side
constexpr let LANES = aoc::constant::md5_lanes;

auto parse(String const& filename) -> String {
  return String{aoc::trim(aoc::read_file(filename))};
}

/// Whether the hash starts with the five or six zeroes the part asks for
template <bool Part2>
fn is_answer(aoc::Digest const& hash) -> bool {
  if ((hash[0] != 0) || (hash[1] != 0)) {
    return false;
  }
  if constexpr (Part2) {
    return hash[2] == 0;
  } else {
    return (hash[2] & 0xF0u) == 0;
  }
}

template <bool Part2>
fn solve_case(str key) -> u32 {
  let num_threads = aoc::num_worker_threads();
  auto result = std::atomic<u32>{std::numeric_limits<u32>::max()};

  // A thread takes a whole batch at a time,
  // so the numbers it hashes side by side are the ones next to each other
  let step = num_threads * static_cast<u32>(LANES);
  auto search = [&, key](u32 start) {
    auto buffers = std::array<std::array<char, 64>, LANES>{};
    for (auto& buffer : buffers) {
      stdr::copy(key, std::begin(buffer));
    }
    auto messages = std::array<str, LANES>{};
    auto hashes = std::array<aoc::Digest, LANES>{};
    for (auto n = start; n < result.load(std::memory_order_relaxed);
         n += step) {
      for (let lane : Range{0uz, LANES}) {
        auto& buffer = buffers[lane];
        let[end, _] =
            std::to_chars(std::begin(buffer) + key.size(), std::end(buffer),
                          n + static_cast<u32>(lane));
        messages[lane] = str{std::begin(buffer), end};
      }
      aoc::md5_many(messages, hashes);

      // The batch runs upwards, so its first answer is its smallest
      for (let lane : Range{0uz, LANES}) {
        if (is_answer<Part2>(hashes[lane])) {
          let answer = n + static_cast<u32>(lane);
          auto current = result.load(std::memory_order_relaxed);
          while ((answer < current) &&
                 !result.compare_exchange_weak(current, answer,
                                               std::memory_order_relaxed))
            ;
          return;
        }
      }
    }
  };

  {
    auto threads = std::vector<std::jthread>{};
    threads.reserve(num_threads);
    for (u32 i = 0; i < num_threads; ++i) {
      threads.emplace_back(search, i * static_cast<u32>(LANES));
    }
  } // all threads join here

  return result.load();
}

int main() {
  std::println("Part 1");
  let example = parse("day04.example");
  AOC_EXPECT_RESULT(609043, solve_case<false>(example));
  AOC_EXPECT_RESULT(1048970, solve_case<false>("pqrstuv"));
  let input = parse("day04.input");
  AOC_EXPECT_RESULT(254575, solve_case<false>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(6742839, solve_case<true>(example));
  AOC_EXPECT_RESULT(5714438, solve_case<true>("pqrstuv"));
  AOC_EXPECT_RESULT(1038736, solve_case<true>(input));

  AOC_RETURN_CHECK_RESULT();
}
