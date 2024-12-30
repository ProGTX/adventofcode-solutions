// https://adventofcode.com/2024/day/13

#include "../common/common.h"

#include <array>
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

struct claw_machine {
  point a;
  point b;
  point prize;

  constexpr int lowest_cost() {
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
    auto A = (prize.y - B * b.y) / a.y;
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

constexpr std::string_view prize_prefix = "Prize: ";
constexpr std::string_view ax_prefix = "A: X+";
constexpr std::string_view y_prefix = " Y+";

template <bool>
int solve_case(const std::string& filename) {
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
    auto [x, y] = aoc::split<std::array<std::string_view, 2>>(line, ',');

    if (prefix == prize_prefix) {
      current.prize = point{aoc::to_number<int>(x.substr(2)),
                            aoc::to_number<int>(y.substr(y_prefix.size()))};
    } else if (line[0] == 'A') {
      current.a = point{aoc::to_number<int>(x.substr(ax_prefix.size())),
                        aoc::to_number<int>(y.substr(y_prefix.size()))};
    } else if (line[0] == 'B') {
      current.b = point{aoc::to_number<int>(x.substr(ax_prefix.size())),
                        aoc::to_number<int>(y.substr(y_prefix.size()))};
    } else {
      AOC_ASSERT(false, "Parsing failure");
    }
  }

  int sum = 0;
  sum = aoc::ranges::accumulate(
      machines | std::views::transform(&claw_machine::lowest_cost), 0);
  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(480, solve_case<false>("day13.example"));
  AOC_EXPECT_RESULT(38714, solve_case<false>("day13.input"));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(281, solve_case<true>("day13.example"));
  // AOC_EXPECT_RESULT(53515, solve_case<true>("day13.input"));
  AOC_RETURN_CHECK_RESULT();
}
