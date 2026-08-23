// https://adventofcode.com/2016/day/5

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <array>
#include <charconv>
#include <print>
#include <thread>
#include <utility>
#endif

using Input = String;

auto parse(String const& filename) -> Input {
  return aoc::read_single_line(filename);
}

/// Num indices each thread scans before the results are merged back in order
constexpr u32 CHUNK = 1u << 16;

/// The 6th and 7th digit of a hash that starts with five zeroes
using Digits = std::pair<u8, u8>;

/// Scans `[base, base + CHUNK * num_threads)` in parallel,
/// collecting the 6th and 7th hash digits of every index
/// whose hash starts with five zeroes, in index order.
auto scan(str door_id, u32 base, u32 num_threads) -> Vec<Digits> {
  auto found = Vec<Vec<Digits>>(num_threads);
  {
    auto threads = Vec<std::jthread>{};
    threads.reserve(num_threads);
    for (let t : Range{0u, num_threads}) {
      threads.emplace_back([&found, door_id, base, t] {
        auto buffer = std::array<char, 32>{};
        stdr::copy(door_id, std::begin(buffer));
        auto& results = found[t];
        let start = base + (t * CHUNK);
        for (let index : Range{start, start + CHUNK}) {
          let[end, _] = std::to_chars(std::begin(buffer) + door_id.size(),
                                      std::end(buffer), index);
          let hash = aoc::md5(str{std::begin(buffer), end});
          if ((hash[0] == 0) && (hash[1] == 0) && ((hash[2] & 0xF0u) == 0)) {
            results.emplace_back(hash[2] & 0x0Fu, hash[3] >> 4);
          }
        }
      });
    }
  } // all threads join here
  return found | stdv::join | aoc::collect_vec<Digits>();
}

template <bool Enhanced>
fn solve_case(Input const& door_id) -> String {
  let num_threads = aoc::num_worker_threads();
  constexpr let PASSLEN = 8uz;
  auto password = String(PASSLEN, ' ');
  auto filled = usize{};
  for (auto base = u32{}; filled < PASSLEN; base += CHUNK * num_threads) {
    for (let[sixth, seventh] : scan(door_id, base, num_threads)) {
      auto pid = filled;
      auto nibble = usize{sixth};
      if constexpr (Enhanced) {
        if ((sixth >= PASSLEN) || (password[sixth] != ' ')) {
          continue;
        }
        pid = sixth;
        nibble = seventh;
      }
      password[pid] = "0123456789abcdef"[nibble];
      ++filled;
      if (filled == PASSLEN) {
        break;
      }
    }
  }
  return password;
}

int main() {
  std::println("Part 1");
  let example = parse("day05.example");
  AOC_EXPECT_RESULT("18f47a30", solve_case<false>(example));
  let input = parse("day05.input");
  AOC_EXPECT_RESULT("c6697b55", solve_case<false>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT("05ace8e3", solve_case<true>(example));
  AOC_EXPECT_RESULT("8c35d1ab", solve_case<true>(input));

  AOC_RETURN_CHECK_RESULT();
}
