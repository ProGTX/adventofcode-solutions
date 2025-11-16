// https://adventofcode.com/2024/day/13

#include "../common/common.h"

#include <array>
#include <cstdint>
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using int_t = std::int64_t;
using point_t = aoc::point_type<int_t>;

struct claw_machine {
  point_t a;
  point_t b;
  point_t prize;

  constexpr int_t lowest_cost() const {
    // prize = c
    // A*ax + B*bx = cx
    // A*ay + B*by = cy
    // Solve A and B

    auto B_num = a.x * prize.y - a.y * prize.x;
    auto B_den = a.x * b.y - a.y * b.x;
    if ((B_den == 0) || ((B_num % B_den) != 0)) {
      return 0;
    }
    auto B = B_num / B_den;

    auto A_num = (prize.y - B * b.y);
    auto A_den = a.y;
    if ((A_den == 0) || ((A_num % A_den) != 0)) {
      return 0;
    }
    auto A = A_num / A_den;

    return A * 3 + B;
  }
};

static_assert(280 ==
              claw_machine{{94, 34}, {22, 67}, {8400, 5400}}.lowest_cost());
static_assert(0 ==
              claw_machine{{26, 66}, {67, 21}, {12748, 12176}}.lowest_cost());
static_assert(200 ==
              claw_machine{{17, 86}, {84, 37}, {7870, 6450}}.lowest_cost());
static_assert(0 ==
              claw_machine{{69, 23}, {27, 71}, {18641, 10279}}.lowest_cost());

constexpr const int_t correction = 10000000000000;

static_assert(0 == claw_machine{{94, 34},
                                {22, 67},
                                {correction + 8400, correction + 5400}}
                       .lowest_cost());
static_assert(459236326669 ==
              claw_machine{
                  {26, 66}, {67, 21}, {correction + 12748, correction + 12176}}
                  .lowest_cost());
static_assert(0 == claw_machine{{17, 86},
                                {84, 37},
                                {correction + 7870, correction + 6450}}
                       .lowest_cost());
static_assert(416082282239 ==
              claw_machine{
                  {69, 23}, {27, 71}, {correction + 18641, correction + 10279}}
                  .lowest_cost());

constexpr const std::string_view prize_prefix = "Prize: ";
constexpr const std::string_view ax_prefix = "A: X+";
constexpr const std::string_view y_prefix = " Y+";

template <bool correct_conversion>
int_t solve_case(const std::string& filename) {
  std::vector<claw_machine> machines;
  machines.emplace_back();

  for (std::string_view line :
       aoc::views::read_lines(filename, aoc::keep_empty{})) {
    if (line.empty()) {
      machines.emplace_back();
      continue;
    }

    auto& current = machines.back();
    const auto prefix = line.substr(0, prize_prefix.size());
    line = line.substr(prize_prefix.size());
    auto [x, y] = aoc::split_once(line, ',');

    if (prefix == prize_prefix) {
      current.prize = point_t{aoc::to_number<int_t>(x.substr(2)),
                              aoc::to_number<int_t>(y.substr(y_prefix.size()))};
      if constexpr (correct_conversion) {
        current.prize += point_t{correction, correction};
      }
    } else if (line[0] == 'A') {
      current.a = point_t{aoc::to_number<int_t>(x.substr(ax_prefix.size())),
                          aoc::to_number<int_t>(y.substr(y_prefix.size()))};
    } else if (line[0] == 'B') {
      current.b = point_t{aoc::to_number<int_t>(x.substr(ax_prefix.size())),
                          aoc::to_number<int_t>(y.substr(y_prefix.size()))};
    } else {
      AOC_ASSERT(false, "Parsing failure");
    }
  }

  int_t sum = 0;
  sum = aoc::ranges::accumulate(
      machines | std::views::transform(&claw_machine::lowest_cost), int_t{0});
  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(480, solve_case<false>("day13.example"));
  AOC_EXPECT_RESULT(38714, solve_case<false>("day13.input"));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT((459236326669 + 416082282239),
                    solve_case<true>("day13.example"));
  AOC_EXPECT_RESULT(74015623345775, solve_case<true>("day13.input"));
  AOC_RETURN_CHECK_RESULT();
}
