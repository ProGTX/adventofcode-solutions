// https://adventofcode.com/2022/day/13

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <print>
#include <ranges>
#include <variant>

namespace stdv = std::views;
namespace stdr = std::ranges;

struct Packet;
using PacketList = Vec<Packet>;
using PacketPair = aoc::point_type<Packet>;
using Input = Vec<PacketPair>;

struct Packet {
  std::variant<u32, PacketList> data;
  constexpr std::strong_ordering operator<=>(Packet const& other) const;
  constexpr bool operator==(Packet const& other) const = default;

  friend std::ostream& operator<<(std::ostream& out, Packet const& p) {
    aoc::match(
        p.data, //
        [&](u32 num) { out << num; },
        [&](PacketList const& list) {
          out << '[';
          for (let& v : list) {
            out << v << ',';
          }
          out << ']';
        });
    return out;
  }
};

fn parse_list_inner(str s) -> Packet {
  auto list_stack = Vec<PacketList>{};
  list_stack.emplace_back();
  auto current_number = Option<u32>{};
  let push_number = [&] {
    if (current_number.has_value()) {
      list_stack.back().emplace_back(*current_number);
    }
    current_number = None;
  };
  for (let c : s) {
    if (std::isdigit(c)) {
      let digit = aoc::to_number<u32>(c);
      if (current_number.has_value()) {
        current_number = *current_number * 10 + digit;
      } else {
        current_number = digit;
      }
    } else {
      switch (c) {
        case '[': {
          push_number();
          list_stack.emplace_back();
        } break;
        case ']': {
          push_number();
          let last = aoc::pop_stack(list_stack);
          list_stack.back().emplace_back(std::move(last));
        } break;
        case ',': {
          push_number();
        } break;
        default:
          AOC_ASSERT(std::isdigit(c), "Invalid character in input");
      }
    }
  }
  push_number();
  return Packet{list_stack.back()};
}

auto parse(String const& filename) -> Input {
  return aoc::split(aoc::trim(aoc::read_file(filename)), "\n\n") |
         stdv::transform([](str pair) {
           let[first, second] = aoc::split_once(pair, '\n');
           return PacketPair{
               parse_list_inner(first.substr(1, first.size() - 2)),
               parse_list_inner(second.substr(1, second.size() - 2)),
           };
         }) |
         aoc::ranges::to<Input>();
}

constexpr std::strong_ordering Packet::operator<=>(Packet const& other) const {
  let overload = aoc::overload{
      [](u32 left, u32 right) { return left <=> right; }, // FORCE FORMATTING
      [](PacketList const& left, PacketList const& right) {
        return std::lexicographical_compare_three_way(
            std::begin(left), std::end(left), //
            std::begin(right), std::end(right));
      },
      [](u32 left, PacketList const& right) {
        return PacketList(1, Packet{left}) <=> right;
      },
      [](PacketList const& left, u32 right) {
        return left <=> PacketList(1, Packet{right});
      }};
  return std::visit(overload, data, other.data);
}

fn solve_case1(Input const& input) -> u32 {
  return aoc::ranges::accumulate(
      input | stdv::enumerate | stdv::filter([](let& index_val_pair) {
        let & [ first, second ] = std::get<1>(index_val_pair);
        return first < second;
      }) | stdv::transform([](let& index_val_pair) {
        return std::get<0>(index_val_pair) + 1;
      }),
      u32{});
}

fn solve_case2(Input const& input) -> u32 {
  auto packets = input | stdv::join | aoc::ranges::to<Vec<Packet>>();
  let p2 = parse_list_inner("[2]");
  let p6 = parse_list_inner("[6]");
  packets.push_back(p2);
  packets.push_back(p6);
  stdr::sort(packets);
  return (aoc::ranges::position(packets, p2).value() + 1) *
         (aoc::ranges::position(packets, p6).value() + 1);
}

int main() {
  std::println("Part 1");
  let example = parse("day13.example");
  AOC_EXPECT_RESULT(13, solve_case1(example));
  let input = parse("day13.input");
  AOC_EXPECT_RESULT(5198, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(140, solve_case2(example));
  AOC_EXPECT_RESULT(22344, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
