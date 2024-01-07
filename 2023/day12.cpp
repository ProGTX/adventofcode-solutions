// https://adventofcode.com/2023/day/12

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <ostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using namespace std::string_view_literals;

constexpr inline char operational = '.';
constexpr inline char damaged = '#';
constexpr inline char unknown = '?';

struct state_t {
  std::string arr;
  int arr_pos;
  int group_id;
  int current_group_count;

  constexpr auto as_array() const {
    return std::array{arr_pos, group_id, current_group_count};
  }

  constexpr bool operator==(const state_t& other) const = default;
};

constexpr std::pair<state_t, char> advance_state(
    state_t state, std::span<const int> spring_groups) {
  AOC_ASSERT(state.current_group_count <= spring_groups[state.group_id],
             "Invalid group count");
  AOC_ASSERT(state.group_id < spring_groups.size(), "Invalid group ID");
  auto previous =
      (state.arr_pos > 0) ? state.arr[state.arr_pos - 1] : operational;
  for (; state.arr_pos < state.arr.size(); ++state.arr_pos) {
    switch (state.arr[state.arr_pos]) {
      case unknown:
        return {state, unknown};
      case damaged:
        if ((previous == damaged) && (state.current_group_count == 0)) {
          return {state, damaged};
        }
        ++state.current_group_count;
        if (state.current_group_count == spring_groups[state.group_id]) {
          state.current_group_count = 0;
          ++state.group_id;
          if (state.group_id == spring_groups.size()) {
            // We don't have any more groups to check,
            // so we need to check if there are any more damaged springs.
            // If there are, the arrangement is invalid.
            // Otherwise, the arrangement is valid.
            auto num_damaged_after =
                std::ranges::count(std::begin(state.arr) + state.arr_pos + 1,
                                   std::end(state.arr), damaged);
            return {state, (num_damaged_after == 0) ? operational : damaged};
          }
        }
        break;
      case operational:
        if (state.current_group_count > 0) {
          return {state, damaged};
        } else {
          // Everything is still OK
          break;
        }
      default:
        AOC_ASSERT(false, "Unexpected state");
        break;
    }
    previous = state.arr[state.arr_pos];
  }
  return {state,
          (state.group_id == spring_groups.size()) ? operational : damaged};
}

#if defined(AOC_COMPILER_GCC)
static_assert(std::pair{state_t{"???.###", 0, 0, 0}, unknown} ==
              advance_state(state_t{"???.###", 0, 0, 0}, std::array{1, 1, 3}));
static_assert(std::pair{state_t{"#??.###", 1, 0, 1}, unknown} ==
              advance_state(state_t{"#??.###", 1, 0, 1}, std::array{1, 1, 3}));
static_assert(std::pair{state_t{"#.#.###", 6, 3, 0}, operational} ==
              advance_state(state_t{"#.#.###", 0, 0, 0}, std::array{1, 1, 3}));
static_assert(std::pair{state_t{"#...###", 5, 2, 0}, damaged} ==
              advance_state(state_t{"#...###", 0, 0, 0}, std::array{1, 1, 3}));
static_assert(std::pair{state_t{".###.##.....", 12, 2, 0}, damaged} ==
              advance_state(state_t{".###.##.....", 0, 0, 0},
                            std::array{3, 2, 1}));
static_assert(std::pair{state_t{".###.#.#...#", 6, 1, 1}, damaged} ==
              advance_state(state_t{".###.#.#...#", 0, 0, 0},
                            std::array{3, 2, 1}));
static_assert(std::pair{state_t{".###.##.#???", 8, 3, 0}, operational} ==
              advance_state(state_t{".###.##.#???", 0, 0, 0},
                            std::array{3, 2, 1}));
#endif

constexpr int num_arrangements(std::string_view springs,
                               std::span<const int> spring_groups) {
  std::vector<state_t> arrangement_stack;
  arrangement_stack.emplace_back(std::string{springs}, 0, 0, 0);
  auto pop_stack = [&]() {
    auto arr_state = std::move(arrangement_stack.back());
    arrangement_stack.resize(arrangement_stack.size() - 1);
    return arr_state;
  };
  int count = 0;
  while (!arrangement_stack.empty()) {
    auto [arr_state, status] = advance_state(pop_stack(), spring_groups);
    if (status == operational) {
      // Valid arrangement
      ++count;
      continue;
    } else if (status == unknown) {
      // Add two more options to the stack
      arr_state.arr[arr_state.arr_pos] = operational;
      state_t arr_state2 = arr_state;
      arr_state2.arr[arr_state2.arr_pos] = damaged;
      arrangement_stack.push_back(std::move(arr_state));
      arrangement_stack.push_back(std::move(arr_state2));
    } else {
      AOC_ASSERT(status == damaged, "Unexpected status");
      // Invalid arrangement, don't count it
    }
  }
  return count;
}

#if defined(AOC_COMPILER_GCC)
static_assert(1 == num_arrangements("???.###", std::array{1, 1, 3}));
static_assert(4 == num_arrangements(".??..??...?##.", std::array{1, 1, 3}));
static_assert(10 == num_arrangements("?###????????", std::array{3, 2, 1}));
#endif

template <int factor>
constexpr std::pair<std::string, std::vector<int>> unfold(
    std::string_view springs, std::span<const int> spring_groups) {
  std::string new_springs{springs};
  std::vector<int> new_groups;
  new_springs.reserve(springs.size() * factor + (factor - 1));
  new_groups.reserve(spring_groups.size() * factor);
  std::ranges::copy(spring_groups, std::back_inserter(new_groups));
  for (int i = 1; i < factor; ++i) {
    new_springs.append("?");
    new_springs.append(springs);
    std::ranges::copy(spring_groups, std::back_inserter(new_groups));
  }
  return {new_springs, new_groups};
}

static_assert(".#?.#?.#?.#?.#" == unfold<5>(".#", std::array{1}).first);
static_assert(std::ranges::equal(std::array{1, 1, 1, 1, 1},
                                 unfold<5>(".#", std::array{1}).second));
static_assert("???.###????.###????.###????.###????.###" ==
              unfold<5>("???.###", std::array{1, 1, 3}).first);
static_assert(
    std::ranges::equal(std::array{1, 1, 3, 1, 1, 3, 1, 1, 3, 1, 1, 3, 1, 1, 3},
                       unfold<5>("???.###", std::array{1, 1, 3}).second));

template <int factor>
int solve_case(const std::string& filename) {
  int sum = 0;
  auto read_values = [&](std::string_view line) {
    auto [springs, groups_str] = split<std::array<std::string, 2>>(line, ' ');
    auto spring_groups = split<std::vector<int>>(groups_str, ',');
    std::tie(springs, spring_groups) = unfold<factor>(springs, spring_groups);
    sum += num_arrangements(springs, spring_groups);
  };
  readfile_op(filename, read_values);

  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(21, (solve_case<1>("day12.example")));
  AOC_EXPECT_RESULT(7379, (solve_case<1>("day12.input")));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(525152, (solve_case<5>("day12.example")));
  // AOC_EXPECT_RESULT(525152, (solve_case<5>("day12.input")));
  AOC_RETURN_CHECK_RESULT();
}
