// https://adventofcode.com/2023/day/3

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <print>
#include <ranges>

auto parse(String const& filename) -> Vec<String> {
  return aoc::read_lines(filename);
}

fn is_symbol(char c) -> bool { return !aoc::is_number(c) && (c != '.'); }

struct number_t {
  int row;
  int column;
  int size;

  bool operator==(const number_t&) const = default;
};

fn find_number(str line, i32 linenum, i32 init_pos) -> Option<number_t> {
  if (!aoc::is_number(line[init_pos])) {
    return std::nullopt;
  }

  number_t number{linenum - 1, 0, 0};
  auto start_pos = init_pos;
  for (; start_pos >= 0; --start_pos) {
    if (!aoc::is_number(line[start_pos])) {
      break;
    }
  }
  // The column is one less than desired after the for loop finishes
  number.column = start_pos + 1;

  auto end_pos = init_pos + 1;
  for (; end_pos < line.size(); ++end_pos) {
    if (!aoc::is_number(line[end_pos])) {
      break;
    }
  }
  number.size = end_pos - number.column;

  return number;
}

static_assert(number_t{0, 0, 3} == *find_number("467..114..", 1, 0));
static_assert(number_t{0, 0, 3} == *find_number("467..114..", 1, 1));
static_assert(number_t{0, 0, 3} == *find_number("467..114..", 1, 2));

template <bool check_gears>
fn solve_case(Vec<String> const& lines) -> i32 {
  using number_pair_t = std::pair<number_t, i32>;
  auto all_numbers = Vec<number_pair_t>{};
  [[maybe_unused]] auto current_numbers = Vec<number_pair_t>{};

  let add_number = [&](str line, i32 linenum, i32 pos) {
    auto number = find_number(line, linenum, pos);
    if (!number) {
      return;
    }
    auto it = stdr::find(all_numbers, *number, &number_pair_t::first);
    if (it != std::end(all_numbers)) {
      if constexpr (check_gears) {
        if (!stdr::contains(current_numbers, it->first,
                            &number_pair_t::first)) {
          current_numbers.emplace_back(it->first, it->second);
        }
      }
      return;
    }
    let number_value =
        aoc::to_number<i32>(line.substr(number->column, number->size));
    all_numbers.emplace_back(*number, number_value);
    if constexpr (check_gears) {
      if (!stdr::contains(current_numbers, *number, &number_pair_t::first)) {
        current_numbers.emplace_back(*number, number_value);
      }
    }
  };

  [[maybe_unused]] auto gear_ratios = Vec<i32>{};
  auto line_minus2 = String{};
  auto line_minus1 = String{};

  for (let[linenum_idx, line] : lines | stdv::enumerate) {
    let linenum = static_cast<i32>(linenum_idx) + 1;
    if (linenum < 3) [[unlikely]] {
      if (linenum == 1) {
        line_minus2 = line;
      } else {
        line_minus1 = line;
      }
      continue;
    }

    // Note that we're cheating a bit here based on the knowledge
    // that the edges don't contain a symbol, so we can get away with
    // just checking the inner values and not doing any bounds checks
    for (let pos : Range{1, static_cast<i32>(line_minus1.size()) - 1}) {
      auto c = line_minus1[pos];
      if (!is_symbol(c)) {
        continue;
      }
      if constexpr (check_gears) {
        current_numbers.clear();
      }
      add_number(line_minus2, linenum - 2, pos - 1);
      add_number(line_minus2, linenum - 2, pos);
      add_number(line_minus2, linenum - 2, pos + 1);
      add_number(line_minus1, linenum - 1, pos - 1);
      add_number(line_minus1, linenum - 1, pos + 1);
      add_number(line, linenum, pos - 1);
      add_number(line, linenum, pos);
      add_number(line, linenum, pos + 1);
      if constexpr (check_gears) {
        if (current_numbers.size() == 2) {
          gear_ratios.push_back(current_numbers[0].second *
                                current_numbers[1].second);
        }
      }
    }
    line_minus2 = line;
    std::swap(line_minus1, line_minus2);
  }

  if constexpr (!check_gears) {
    return aoc::ranges::accumulate(
        all_numbers | stdv::transform(&number_pair_t::second), i32{});
  } else {
    return aoc::ranges::accumulate(gear_ratios, i32{});
  }
}

int main() {
  std::println("Part 1");
  let example = parse("day03.example");
  AOC_EXPECT_RESULT(4361, solve_case<false>(example));
  let input = parse("day03.input");
  AOC_EXPECT_RESULT(537732, solve_case<false>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(467835, solve_case<true>(example));
  AOC_EXPECT_RESULT(84883664, solve_case<true>(input));

  AOC_RETURN_CHECK_RESULT();
}
