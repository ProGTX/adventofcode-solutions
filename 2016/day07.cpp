// https://adventofcode.com/2016/day/7

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <print>
#include <tuple>
#include <utility>
#endif

struct IPv7 {
  Vec<String> inside;
  Vec<String> outside;
};

using Input = Vec<IPv7>;

auto parse(String const& filename) -> Input {
  return aoc::views::read_lines(filename) |
         stdv::transform([](str line) {
           let parts = aoc::split_to_vec<str>(line, '[');
           auto packet = IPv7{};
           packet.outside.emplace_back(parts[0]);
           for (let part : parts | stdv::drop(1)) {
             let[inner, outer] = aoc::split_once(part, ']');
             packet.inside.emplace_back(inner);
             packet.outside.emplace_back(outer);
           }
           return packet;
         }) |
         aoc::collect_vec<IPv7>();
}

fn abba(str sequence) -> bool {
  return stdr::any_of(sequence | stdv::adjacent<4>, [](let& window) {
    let[a, b, c, d] = window;
    return (a == d) && (b == c) && (a != b);
  });
}

fn solve_case1(Input const& packets) -> usize {
  return static_cast<usize>(stdr::count_if(packets, [](IPv7 const& packet) {
    return stdr::any_of(packet.outside, abba) &&
           !stdr::any_of(packet.inside, abba);
  }));
}

fn aba_list(str sequence) {
  return sequence |
         stdv::adjacent<3> |
         aoc::views::transform_filter(
             [](let& window) -> Option<std::pair<char, char>> {
               let[a, b, c] = window;
               if ((a != c) || (a == b)) {
                 return None;
               }
               return std::pair{a, b};
             });
}

fn has_bab(Vec<String> const& sequences, char a, char b) -> bool {
  return stdr::any_of(sequences, [a, b](str sequence) {
    return stdr::any_of(sequence | stdv::adjacent<3>, [a, b](let& window) {
      return window == std::tuple{b, a, b};
    });
  });
}

fn solve_case2(Input const& packets) -> usize {
  return static_cast<usize>(stdr::count_if(packets, [](IPv7 const& packet) {
    return stdr::any_of(packet.outside, [&](str outer) {
      return stdr::any_of(aba_list(outer), [&](let& aba) {
        return has_bab(packet.inside, aba.first, aba.second);
      });
    });
  }));
}

int main() {
  std::println("Part 1");
  let example = parse("day07.example");
  AOC_EXPECT_RESULT(2, solve_case1(example));
  let example2 = parse("day07.example2");
  AOC_EXPECT_RESULT(0, solve_case1(example2));
  let input = parse("day07.input");
  AOC_EXPECT_RESULT(110, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(0, solve_case2(example));
  AOC_EXPECT_RESULT(3, solve_case2(example2));
  AOC_EXPECT_RESULT(242, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
