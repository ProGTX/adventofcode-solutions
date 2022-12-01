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

void solve_case(const std::string& filename, int window_width) {
  auto measurements = readfile_numbers(filename);
  const auto displacement = window_width - 1;

  const auto it = std::begin(measurements);
  for (int i = displacement; i < measurements.size(); ++i) {
    auto window_start = it + i - displacement;
    auto window_end = window_start + window_width;
    auto sum = std::accumulate(window_start, window_end, 0);
    // Inline modification
    *window_start = sum;
  }

  namespace views = std::ranges::views;

  std::vector<int> diffs;
  int previousMeasure = 0;
  const auto diffFunc = [&](int currentMeasure) {
    diffs.push_back(currentMeasure - previousMeasure);
    previousMeasure = currentMeasure;
  };
  std::ranges::for_each(
      measurements | views::take(measurements.size() - displacement), diffFunc);

  auto relevantDiffs = diffs | views::drop(1);
  auto count =
      std::ranges::count_if(relevantDiffs, [](int diff) { return diff > 0; });

  std::cout << filename << " -> " << count << std::endl;
}

int main() {
  std::cout << "Part 1" << std::endl;
  solve_case("day01.example", 1);
  solve_case("day01.input", 1);
  std::cout << "Part 2" << std::endl;
  solve_case("day01.example", 3);
  solve_case("day01.input", 3);
}
