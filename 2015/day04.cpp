// https://adventofcode.com/2015/day/4

#include "../common/common.h"
#include "../common/rust.h"

#include <atomic>
#include <charconv>
#include <cstring>
#include <limits>
#include <print>
#include <thread>
#include <vector>

auto parse(String const& filename) -> String {
  return String{aoc::trim(aoc::read_file(filename))};
}

template <bool Part2>
fn solve_case(str key) -> u32 {
  let num_threads = std::max(1u, std::thread::hardware_concurrency());
  auto result = std::atomic<u32>{std::numeric_limits<u32>::max()};

  auto search = [&, key](u32 start) {
    char buf[64];
    std::memcpy(buf, key.data(), key.size());
    for (auto n = start; n < result.load(std::memory_order_relaxed);
         n += num_threads) {
      let[end, _] = std::to_chars(buf + key.size(), buf + sizeof(buf), n);
      let hash = aoc::md5(str{buf, end});
      if (hash[0] == 0 && hash[1] == 0 && (hash[2] & 0xF0u) == 0) {
        if constexpr (Part2) {
          if ((hash[2] & 0x0Fu) > 0) {
            continue;
          }
        }
        auto current = result.load(std::memory_order_relaxed);
        while ((n < current) && !result.compare_exchange_weak(
                                    current, n, std::memory_order_relaxed))
          ;
        return;
      }
    }
  };

  {
    auto threads = std::vector<std::jthread>{};
    threads.reserve(num_threads);
    for (u32 i = 0; i < num_threads; ++i) {
      threads.emplace_back(search, i);
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
