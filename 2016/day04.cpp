// https://adventofcode.com/2016/day/4

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <array>
#include <numeric>
#include <print>
#endif

// The name keeps the dashes that separate its words
constexpr let SEPARATOR = '-';

constexpr let NUM_LETTERS = 26uz;
constexpr let CHECKSUM_LENGTH = 5uz;

struct Room {
  String name;
  usize sector;
  std::array<char, CHECKSUM_LENGTH> checksum;
};

using Input = Vec<Room>;

auto parse(String const& filename) -> Input {
  return aoc::views::read_lines(filename) |
         stdv::transform([](str line) {
           let tail_start = line.rfind(SEPARATOR) + 1;
           let[sector, checksum] =
               aoc::split_once(line.substr(tail_start), '[');
           auto room = Room{
               .name = String{line.substr(0, tail_start - 1)},
               .sector = aoc::to_number<usize>(sector),
               .checksum = {},
           };
           // The checksum still carries the closing bracket
           stdr::copy_n(checksum.begin(), CHECKSUM_LENGTH,
                        room.checksum.begin());
           return room;
         }) |
         aoc::collect_vec<Room>();
}

fn solve_case1(Input const& rooms) -> usize {
  auto sum = usize{};
  for (let& room : rooms) {
    auto frequency = std::array<usize, NUM_LETTERS>{};
    for (let letter : room.name) {
      if (letter != SEPARATOR) {
        ++frequency[static_cast<usize>(letter - 'a')];
      }
    }
    auto sorted_ids = aoc::views::indices_of(frequency) |
                      aoc::collect_static_vec<u8, NUM_LETTERS>();
    stdr::sort(sorted_ids, [&](u8 lhs, u8 rhs) {
      return (frequency[lhs] != frequency[rhs])
                 ? (frequency[lhs] > frequency[rhs])
                 : (lhs < rhs);
    });
    if (stdr::equal(       //
            room.checksum, //
            sorted_ids |
                stdv::take(CHECKSUM_LENGTH) |
                stdv::transform([](u8 letter_id) {
                  return static_cast<char>(letter_id + 'a');
                }))) {
      sum += room.sector;
    }
  }
  return sum;
}

fn solve_case2(Input const& rooms) -> usize {
  for (let& room : rooms) {
    let real_name =
        room.name |
        stdv::transform([&](char letter) {
          if (letter == SEPARATOR) {
            return ' ';
          }
          let rotated =
              (static_cast<usize>(letter - 'a') + room.sector) % NUM_LETTERS;
          return static_cast<char>(rotated + 'a');
        }) |
        aoc::collect_string();
    if (real_name == "northpole object storage") {
      return room.sector;
    }
  }
  return 0;
}

int main() {
  std::println("Part 1");
  let example = parse("day04.example");
  AOC_EXPECT_RESULT(1514, solve_case1(example));
  let input = parse("day04.input");
  AOC_EXPECT_RESULT(173787, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(0, solve_case2(example));
  AOC_EXPECT_RESULT(548, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
