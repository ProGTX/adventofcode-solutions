// https://adventofcode.com/2022/day/1

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

void solve_case(const std::string& filename, int numElvesAverage) {
  std::vector<int> caloriesPerElf;
  int singleElfCalories = 0;

  auto elfSolver = [&](std::string_view line) {
    if (line.empty()) {
      // New elf, add previous one
      caloriesPerElf.push_back(singleElfCalories);
      singleElfCalories = 0;
      return;
    }
    // Same elf
    int currentCal = std::stoi(std::string{line});
    singleElfCalories += currentCal;
  };

  readfile_op(filename, elfSolver);
  // Need to add last elf
  caloriesPerElf.push_back(singleElfCalories);

  std::ranges::sort(caloriesPerElf);

  namespace views = std::ranges::views;
  auto calsPerTopElves =
      caloriesPerElf | views::reverse | views::take(numElvesAverage);

  auto caloriesSum = std::accumulate(std::begin(calsPerTopElves),
                                     std::end(calsPerTopElves), 0);

  std::cout << filename << " -> " << caloriesSum << std::endl;
}

int main() {
  std::cout << "Part 1" << std::endl;
  solve_case("day01.example", 1);
  solve_case("day01.input", 1);
  std::cout << "Part 2" << std::endl;
  solve_case("day01.example", 3);
  solve_case("day01.input", 3);
}
