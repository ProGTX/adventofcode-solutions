#include <algorithm>
#include <concepts>
#include <fstream>
#include <iostream>
#include <iterator>
#include <numeric>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

// https://stackoverflow.com/a/63050738/793006
constexpr std::string_view ltrim(std::string_view str) {
  const auto pos(str.find_first_not_of(" \t\n\r\f\v"));
  str.remove_prefix(std::min(pos, str.length()));
  return str;
}
constexpr std::string_view rtrim(std::string_view str) {
  const auto pos(str.find_last_not_of(" \t\n\r\f\v"));
  str.remove_suffix(std::min(str.length() - pos - 1, str.length()));
  return str;
}
constexpr std::string_view trim(std::string_view str) {
  str = ltrim(str);
  str = rtrim(str);
  return str;
}

template <std::invocable<std::string_view, int> OpT>
void readfile_op(const std::string& filename, OpT operation) {
  std::ifstream file{filename};
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open file " + filename);
  }
  std::string line;
  for (int linenum = 0; std::getline(file, line); ++linenum) {
    operation(trim(line), linenum);
  }
  file.close();
}

void solve_case(const std::string& filename, int numElvesAverage) {
  std::vector<int> caloriesPerElf;
  int singleElfCalories = 0;

  auto elfSolver = [&](std::string_view line, int linenum) {
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

int main(int argc, char** argv) {
  std::cout << "Part 1" << std::endl;
  solve_case("day01.example", 1);
  solve_case("day01.input", 1);
  std::cout << "Part 2" << std::endl;
  solve_case("day01.example", 3);
  solve_case("day01.input", 3);
}
