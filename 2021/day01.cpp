// https://adventofcode.com/2021/day/1

#include <algorithm>
#include <iostream>
#include <iterator>
#include <numeric>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include "../common.h"

void solve_case(const std::string& filename, int num_lines) {
  std::vector<int> diffs;
  int previousMeasure = 0;

  auto solver = [&](std::string_view line, int linenum) {
    int currentMeasure = std::stoi(std::string{line});
    diffs.push_back(currentMeasure - previousMeasure);
    previousMeasure = currentMeasure;
  };
  readfile_op(filename, solver);

  namespace views = std::ranges::views;
  auto relevantDiffs = diffs | views::drop(1);
  auto count =
      std::ranges::count_if(relevantDiffs, [](int diff) { return diff > 0; });

  std::cout << filename << " -> " << count << std::endl;
}

int main(int argc, char** argv) {
  std::cout << "Part 1" << std::endl;
  solve_case("day01.example", 1);
  solve_case("day01.input", 1);
  std::cout << "Part 2" << std::endl;
  solve_case("day01.example", 3);
  solve_case("day01.input", 3);
}
