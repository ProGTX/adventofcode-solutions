// https://adventofcode.com/2022/day/6

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <exception>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <numeric>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

template <int window_size>
bool all_different(std::string_view window) {
  for (int i = 0; i < window_size - 1; ++i) {
    auto current = window[i];
    auto count = std::ranges::count(window | std::views::drop(i + 1), current);
    if (count > 0) {
      // Element repeats
      return false;
    }
  }
  return true;
}

template <int window_size>
void solve_case(const std::string& filename) {
  std::vector<int> positions;

  readfile_op(filename, [&](std::string_view line) {
    for (int pos = window_size - 1; pos < line.size(); ++pos) {
      int start = pos - window_size + 1;
      auto window = line.substr(start, window_size);
      if (all_different<window_size>(window)) {
        positions.push_back(pos + 1);
        return;
      }
    }
  });

  std::cout << filename << " -> " << std::endl;
  for (auto pos : positions) {
    std::cout << "  " << pos << std::endl;
  }
  std::cout << std::endl;
}

int main() {
  std::cout << "Part 1" << std::endl;
  solve_case<4>("day06.example");
  solve_case<4>("day06.input");
  std::cout << "Part 2" << std::endl;
  solve_case<14>("day06.example");
  solve_case<14>("day06.input");
}
