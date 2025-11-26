// https://adventofcode.com/2015/day/15

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>
#include <span>

struct Ingredient {
  std::array<i32, 4> properties;
  u32 calories;
};

fn parse_property(str property) -> i32 {
  return aoc::to_number<i32>(aoc::split_once(property, ' ')[1]);
}

fn parse(String const& filename) -> Vec<Ingredient> {
  return aoc::views::read_lines(filename) |
         std::views::transform([](str line) {
           let properties = aoc::split_to_array<5>(line, ", ");
           return Ingredient{
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
         aoc::ranges::to<Vec<Ingredient>>();
}

template <bool KCAL_500>
fn score_cookie(std::span<const Ingredient> ingredients,
                std::span<const u32> teaspoon_stack) -> u32 {
  auto result = Ingredient{};
  for (let[index, teaspoon_u32] : teaspoon_stack | std::views::enumerate) {
    let teaspoon = static_cast<i32>(teaspoon_u32);
    for (auto&& [res_prop, cookie_prop] :
         std::views::zip(result.properties, ingredients[index].properties)) {
      res_prop += cookie_prop * teaspoon;
    }
    if constexpr (KCAL_500) {
      result.calories += ingredients[index].calories * teaspoon_u32;
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
  let ingredients = parse(filename);
  auto max_score = 0u;
  for (let& teaspoon_stack : ingredients | aoc::views::counted_combinations(
                                               aoc::combinations_args<u32>{
                                                   .single_min = 0,
                                                   .single_max = 100,
                                                   .all_min = 100,
                                                   .all_max = 100,
                                               })) {
    let score = score_cookie<KCAL_500>(ingredients, teaspoon_stack);
    max_score = std::max(max_score, score);
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
