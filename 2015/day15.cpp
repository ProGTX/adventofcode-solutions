// https://adventofcode.com/2015/day/15

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>
#include <span>

struct Cookie {
  std::array<i32, 4> properties;
  u32 calories;
};

fn parse_property(str property) -> i32 {
  return aoc::to_number<i32>(aoc::split_once(property, ' ')[1]);
}

fn parse(String const& filename) -> Vec<Cookie> {
  return aoc::views::read_lines(filename) |
         std::views::transform([](str line) {
           let properties = aoc::split_to_array<5>(line, ", ");
           return Cookie{
               .properties =
                   {
                       parse_property(aoc::split_once(properties[0], ": ")[1]),
                       parse_property(properties[1]),
                       parse_property(properties[2]),
                       parse_property(properties[3]),
                   },
               .calories = static_cast<u32>(parse_property(properties[4])),
           };
         }) |
         aoc::ranges::to<Vec<Cookie>>();
}

template <bool KCAL_500>
fn score_cookie(std::span<const Cookie> cookies,
                std::span<const u32> teaspoon_stack) -> u32 {
  auto result = Cookie{};
  for (let[index, teaspoon_u32] : teaspoon_stack | std::views::enumerate) {
    let teaspoon = static_cast<i32>(teaspoon_u32);
    for (auto&& [res_prop, cookie_prop] :
         std::views::zip(result.properties, cookies[index].properties)) {
      res_prop += cookie_prop * teaspoon;
    }
    if constexpr (KCAL_500) {
      result.calories += cookies[index].calories * teaspoon_u32;
    }
  }
  if constexpr (KCAL_500) {
    if (result.calories != 500) {
      return 0;
    }
  }
  return std::ranges::fold_left(
      result.properties | std::views::transform([](i32& prop) {
        return static_cast<u32>(std::max(prop, 0));
      }),
      1, std::multiplies{});
}

template <bool KCAL_500>
fn solve_case(String const& filename) -> u32 {
  constexpr let LIMIT = 100u;
  let cookies = parse(filename);
  auto max_score = 0u;
  auto teaspoon_stack = Vec<u32>{};
  teaspoon_stack.push_back(0u);
  auto teaspoons = 0u;
  while (!teaspoon_stack.empty()) {
    if (teaspoons > LIMIT) {
      {
        let last_teaspoon = teaspoon_stack.back();
        teaspoon_stack.pop_back();
        teaspoons -= last_teaspoon;
      }
      if (!teaspoon_stack.empty()) {
        auto& last_teaspoon = teaspoon_stack.back();
        last_teaspoon += 1;
        teaspoons += 1;
        continue;
      } else {
        break;
      }
    }
    if (teaspoon_stack.size() == (cookies.size() - 1)) {
      teaspoon_stack.push_back(LIMIT - teaspoons);
      let score = score_cookie<KCAL_500>(cookies, teaspoon_stack);
      max_score = std::max(max_score, score);
      teaspoon_stack.pop_back();
      teaspoon_stack.back() += 1;
      teaspoons += 1;
      continue;
    }
    teaspoon_stack.push_back(0);
  }
  return max_score;
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(62842880, solve_case<false>("day15.example"));
  AOC_EXPECT_RESULT(21367368, solve_case<false>("day15.input"));
  std::println("Part 2");
  AOC_EXPECT_RESULT(57600000, solve_case<true>("day15.example"));
  AOC_EXPECT_RESULT(1766400, solve_case<true>("day15.input"));
  AOC_RETURN_CHECK_RESULT();
}
