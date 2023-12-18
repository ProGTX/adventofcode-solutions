// https://adventofcode.com/2023/day/9

#include "../common/common.h"

#include <iostream>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using namespace std::string_view_literals;
using int_t = int;

constexpr int predict_value(std::vector<int> reading) {
  std::vector<int> last_values;
  std::vector<int> next_reading;
  int size = reading.size();
  last_values.reserve(size);
  next_reading.reserve(size);
  last_values.push_back(reading.back());
  while (size > 0) {
    next_reading.clear();
    for (int i = 0; i < size - 1; ++i) {
      next_reading.push_back(reading[i + 1] - reading[i]);
    }
    if (std::ranges::all_of(next_reading,
                            [](int value) { return value == 0; })) {
      break;
    }
    last_values.push_back(next_reading.back());
    std::swap(next_reading, reading);
    --size;
  }
  int prediction = 0;
  for (int i = last_values.size() - 1; i >= 0; --i) {
    prediction += last_values[i];
  }
  return prediction;
}

static_assert(18 == predict_value({0, 3, 6, 9, 12, 15}));
static_assert(28 == predict_value({1, 3, 6, 10, 15, 21}));
static_assert(68 == predict_value({10, 13, 16, 21, 30, 45}));

constexpr int sum_predictions(const std::vector<std::vector<int>>& values) {
  return ranges::fold_left(
      values | std::views::transform([](const std::vector<int>& reading) {
        return predict_value(reading);
      }),
      int_t{0}, std::plus<>{});
}

template <bool>
int solve_case(const std::string& filename) {
  std::vector<std::vector<int>> values;
  auto read_values = [&](std::string_view line) {
    values.push_back(split<std::vector<int>>(line, ' '));
  };
  readfile_op(filename, read_values);

  int sum = sum_predictions(values);
  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(114, (solve_case<false>("day09.example")));
  AOC_EXPECT_RESULT(1980437560, (solve_case<false>("day09.input")));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(5905, (solve_case<true>("day09.example")));
  // AOC_EXPECT_RESULT(249776650, (solve_case<true>("day09.input")));
  // AOC_RETURN_CHECK_RESULT();
}
