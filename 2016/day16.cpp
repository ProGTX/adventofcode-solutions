// https://adventofcode.com/2016/day/16

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <print>
#endif

/// The initial state, one bit per element
using Input = Vec<u8>;

fn parse_data(str data) -> Input {
  return data |
         stdv::transform([](char c) { return static_cast<u8>(c - '0'); }) |
         aoc::collect_vec<u8>();
}

auto parse(String const& filename) -> Input {
  return parse_data(aoc::trim(aoc::read_file(filename)));
}

/// Grow the data until it fills `length` bits, then cut off the excess
///
/// One step of the dragon curve is the data so far, a single 0,
/// then the data so far reversed and with every bit flipped
fn dragon_curve(Input const& data, usize length) -> Vec<u8> {
  // The whole disk is allocated up front,
  // and no step is ever grown past it
  auto result = Vec<u8>{};
  result.reserve(std::max(length, data.size()));
  result.insert(result.end(), data.begin(), data.end());
  while (result.size() < length) {
    let old_len = result.size();
    result.push_back(0);
    for (let index : Range{0uz, old_len} | stdv::reverse) {
      if (result.size() == length) {
        break;
      }
      result.push_back(static_cast<u8>(1 - result[index]));
    }
  }
  result.resize(length);
  return result;
}

/// Repeatedly pair up the bits, keeping a 1 where the pair matches,
/// until the result has an odd length
fn checksum(Vec<u8> data) -> Vec<u8> {
  // Each round is written over the front of the same buffer
  while ((data.size() % 2) == 0) {
    let half = data.size() / 2;
    for (let index : Range{0uz, half}) {
      data[index] = static_cast<u8>(data[2 * index] == data[(2 * index) + 1]);
    }
    data.resize(half);
  }
  return data;
}

/// The bits as the characters they are printed as
fn to_string(Vec<u8> const& data) -> String {
  return data |
         stdv::transform([](u8 bit) { return static_cast<char>(bit + '0'); }) |
         aoc::collect_string();
}

static_assert("1" == to_string(parse_data("1")));
static_assert("100" == to_string(dragon_curve(parse_data("1"), 3)));
static_assert("001" == to_string(dragon_curve(parse_data("0"), 3)));
static_assert("11111000000" ==
              to_string(dragon_curve(parse_data("11111"), 11)));
static_assert("1111000010100101011110000" ==
              to_string(dragon_curve(parse_data("111100001010"), 25)));

template <usize DISK_LENGTH>
fn solve_case(Input const& data) -> String {
  // The checksum of the disk once it is filled with random-looking data
  return to_string(checksum(dragon_curve(data, DISK_LENGTH)));
}

int main() {
  std::println("Part 1");
  let example = parse("day16.example");
  AOC_EXPECT_RESULT("01100", solve_case<20>(example));
  AOC_EXPECT_RESULT("11010011110011010", solve_case<272>(example));
  let input = parse("day16.input");
  AOC_EXPECT_RESULT("10100011010101011", solve_case<272>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT("10111110011110111", solve_case<35651584>(example));
  AOC_EXPECT_RESULT("01010001101011001", solve_case<35651584>(input));

  AOC_RETURN_CHECK_RESULT();
}
