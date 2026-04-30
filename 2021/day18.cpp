// https://adventofcode.com/2021/day/18

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <print>
#include <ranges>
#include <span>
#include <string>
#include <variant>

namespace stdr = std::ranges;
namespace stdv = std::views;

// Snailfish number
struct SFN {
  struct Pair {
    std::unique_ptr<SFN> left;
    std::unique_ptr<SFN> right;
  };
  std::variant<u8, Pair> data;

  fn static number(u8 n) { return SFN{n}; }
  fn static pair(SFN left, SFN right) {
    return SFN{Pair{std::make_unique<SFN>(std::move(left)),
                    std::make_unique<SFN>(std::move(right))}};
  }

  fn is_number() const { return std::holds_alternative<u8>(data); }
  fn as_number() -> u8& { return std::get<u8>(data); }
  fn as_number() const -> u8 { return std::get<u8>(data); }
  fn as_pair() -> Pair& { return std::get<Pair>(data); }
  fn as_pair() const -> Pair const& { return std::get<Pair>(data); }

  fn clone() const -> SFN {
    if (is_number()) {
      return number(as_number());
    }
    auto& p = as_pair();
    return pair(p.left->clone(), p.right->clone());
  }
};

// Recursive descent parser
fn parse_sfn_inner(str s) -> std::pair<SFN, str> {
  if (s.starts_with('[')) {
    auto [left, rest] = parse_sfn_inner(s.substr(1));
    auto [right, rest2] = parse_sfn_inner(rest.substr(1)); // skip ','
    return {SFN::pair(std::move(left), std::move(right)),
            rest2.substr(1)}; // skip ']'
  } else {
    auto end = s.find_first_not_of("0123456789");
    if (end == str::npos) {
      end = s.size();
    }
    return {SFN::number(aoc::to_number<u8>(s.substr(0, end))), s.substr(end)};
  }
}
fn parse_sfn(str s) -> SFN { return parse_sfn_inner(s).first; }

auto parse(String const& filename) -> Vec<SFN> {
  return aoc::views::read_lines(filename) |
         stdv::transform(parse_sfn) |
         aoc::ranges::to<Vec<SFN>>();
}

enum class Direction {
  Left,
  Right,
};

template <Direction DIRECTION>
fn add_closest(SFN& sfn, u8 explosive) -> bool {
  if (sfn.is_number()) {
    sfn.as_number() += explosive;
    return true;
  }
  auto& pair = sfn.as_pair();
  if constexpr (DIRECTION == Direction::Left) {
    return add_closest<DIRECTION>(*pair.left, explosive) ||
           add_closest<DIRECTION>(*pair.right, explosive);
  } else {
    return add_closest<DIRECTION>(*pair.right, explosive) ||
           add_closest<DIRECTION>(*pair.left, explosive);
  }
}

struct ExplodeResult {
  Option<u8> left;
  Option<u8> right;
  bool exploded = false;
};

fn explode(SFN& sfn, int level) -> ExplodeResult {
  if (sfn.is_number()) {
    return {};
  }
  auto& pair = sfn.as_pair();
  if (level > 4) {
    let lnum = pair.left->as_number();
    let rnum = pair.right->as_number();
    sfn = SFN::number(0);
    return {lnum, rnum, true};
  }
  auto left_result = explode(*pair.left, level + 1);
  if (left_result.exploded) {
    if (left_result.right.has_value() &&
        add_closest<Direction::Left>(*pair.right, *left_result.right)) {
      return {left_result.left, None, true};
    }
    return left_result;
  }
  auto right_result = explode(*pair.right, level + 1);
  if (right_result.exploded) {
    if (right_result.left.has_value() &&
        add_closest<Direction::Right>(*pair.left, *right_result.left)) {
      return {None, right_result.right, true};
    }
    return right_result;
  }
  return {};
}

fn split(SFN& sfn) -> bool {
  if (sfn.is_number()) {
    let num = sfn.as_number();
    if (num >= 10) {
      sfn = SFN::pair(SFN::number(num / 2), SFN::number(num / 2 + num % 2));
      return true;
    }
    return false;
  }
  auto& pair = sfn.as_pair();
  return split(*pair.left) || split(*pair.right);
}

fn reduce(SFN& sfn) {
  loop {
    while (explode(sfn, 1).exploded) {
    }
    if (!split(sfn)) {
      break;
    }
  }
}

fn add(SFN const& lhs, SFN const& rhs) -> SFN {
  auto result = SFN::pair(lhs.clone(), rhs.clone());
  reduce(result);
  return result;
}

fn magnitude(SFN const& sfn) -> u32 {
  if (sfn.is_number()) {
    return sfn.as_number();
  }
  let& pair = sfn.as_pair();
  return 3 * magnitude(*pair.left) + 2 * magnitude(*pair.right);
}

fn solve_case1(std::span<const SFN> numbers) -> u32 {
  return magnitude(
      stdr::fold_left(numbers.subspan(1), numbers[0].clone(),
                      [](SFN acc, const SFN& rhs) { return add(acc, rhs); }));
}

fn solve_case2(std::span<const SFN> numbers) -> u32 {
  let indices = stdv::iota(usize{0}, numbers.size());
  return stdr::max(
      stdv::cartesian_product(indices, indices) |
      stdv::filter([](auto ij) { return std::get<0>(ij) != std::get<1>(ij); }) |
      stdv::transform([&](auto ij) {
        return magnitude(
            add(numbers[std::get<0>(ij)], numbers[std::get<1>(ij)]));
      }));
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(143, magnitude(parse_sfn("[[1,2],[[3,4],5]]")));
  AOC_EXPECT_RESULT(1384,
                    magnitude(parse_sfn("[[[[0,7],4],[[7,8],[6,0]]],[8,1]]")));
  AOC_EXPECT_RESULT(445, magnitude(parse_sfn("[[[[1,1],[2,2]],[3,3]],[4,4]]")));
  AOC_EXPECT_RESULT(791, magnitude(parse_sfn("[[[[3,0],[5,3]],[4,4]],[5,5]]")));
  AOC_EXPECT_RESULT(1137,
                    magnitude(parse_sfn("[[[[5,0],[7,4]],[5,5]],[6,6]]")));
  AOC_EXPECT_RESULT(
      3488, magnitude(parse_sfn(
                "[[[[8,7],[7,7]],[[8,6],[7,7]]],[[[0,7],[6,6]],[8,7]]]")));
  let example = parse("day18.example");
  AOC_EXPECT_RESULT(4140, solve_case1(example));
  let input = parse("day18.input");
  AOC_EXPECT_RESULT(4207, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(3993, solve_case2(example));
  AOC_EXPECT_RESULT(4635, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
