// https://adventofcode.com/2015/day/11

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <print>
#include <ranges>

fn to_numbers(str const& s) -> Vec<u32> {
  return s |
         std::views::transform(
             [](char c) { return static_cast<u32>(c - 'a'); }) |
         aoc::ranges::to<Vec<u32>>();
}
constexpr const auto I_NUMBER = static_cast<u32>('i' - 'a');
constexpr const auto L_NUMBER = static_cast<u32>('l' - 'a');
constexpr const auto O_NUMBER = static_cast<u32>('o' - 'a');
constexpr const auto Y_NUMBER = static_cast<u32>('y' - 'a');
constexpr const auto Z_NUMBER = static_cast<u32>('z' - 'a');

fn is_invalid_number(u32 n) -> bool {
  return std::ranges::contains(std::array{I_NUMBER, L_NUMBER, O_NUMBER}, n);
}

fn to_string(std::span<const u32> numbers) -> String {
  return numbers |
         std::views::transform([](u32 n) {
           AOC_ASSERT(n <= Z_NUMBER, "Number does not represent a letter");
           return n + 'a';
         }) |
         aoc::ranges::to<String>();
}

// After calling this function we know we don't have any invalid letters
fn remove_initial_invalid(std::span<u32> password) -> bool {
  let invalid_it = std::ranges::find_if(password, &is_invalid_number);
  if (invalid_it == std::end(password)) {
    // Nothing to do
    return false;
  }
  // Increase the invalid letter and zero out the rest
  let invalid_index = std::distance(std::begin(password), invalid_it);
  AOC_ASSERT(!is_invalid_number(Z_NUMBER), "z can't be invalid");
  password[invalid_index] += 1;
  AOC_ASSERT(!is_invalid_number(password[invalid_index]),
             "Invalid letters must be at least 1 apart");
  std::ranges::fill(password.subspan(invalid_index + 1), 0);
  return true;
}

fn increasing_straight_index(std::span<const u32> password)
    -> std::pair<usize, usize> {
  if (password.size() < 3) {
    return {0, 0};
  }
  auto index = 0u;
  auto size = 0u;
  for (let i : std::views::iota(0u, password.size())) {
    auto current_letter = password[i];
    if (std::ranges::contains(std::array{Y_NUMBER, Z_NUMBER}, current_letter)) {
      continue;
    }
    auto current_size = 1;
    for (let j : std::views::iota(i + 1, password.size())) {
      current_letter += 1;
      if (current_letter != password[j]) {
        break;
      }
      current_size += 1;
      if (current_letter == Z_NUMBER) {
        break;
      }
    }
    if (current_size >= 3) {
      index = i;
      size = current_size;
      break;
    }
  }
  return {index, size};
}

fn has_increasing_straight(std::span<const u32> password) -> bool {
  let[index, size] = increasing_straight_index(password);
  if (size < 3) {
    return false;
  }
  let[_, size2] = increasing_straight_index(password.subspan(index + size));
  return (size2 == 0);
}

fn has_letter_pairs(std::span<const u32> password) -> bool {
  let letter_pair_index = [](std::span<const u32> password,
                             usize start) -> Option<usize> {
    AOC_ASSERT(start < password.size() - 1, "Can't search for pairs");
    for (let index : std::views::iota(start, password.size() - 1)) {
      if (password[index] == password[index + 1]) {
        return index;
      }
    }
    return std::nullopt;
  };
  let first_index = letter_pair_index(password, 0);
  if (!first_index.has_value()) {
    return false;
  }
  auto start = *first_index;
  let first_pair_letter = password[start];
  // Search until we find a pair of different letters
  start += 2;
  while (start < (password.size() - 1)) {
    let second_index = letter_pair_index(password, start);
    if (!second_index.has_value()) {
      return false;
    }
    start = *second_index;
    if (password[start] != first_pair_letter) {
      return true;
    }
    start += 2;
  }
  return false;
}

fn is_valid_password(std::span<const u32> password) -> bool {
  // 1. Exactly one increasing straight of at least three letters
  // 2. i, o, l are not allowed -> this is checked elsewhere
  // 3. At least two different non-overlapping pairs of letters
  return has_increasing_straight(password) && has_letter_pairs(password);
}

fn next_password(Vec<u32> password) -> Vec<u32> {
  let size = password.size();
  let increase_by_one = [&](std::span<u32> next) {
    // Try increasing by one at the end of the password
    for (let index : std::views::iota(0u, size) | std::views::reverse) {
      if (next[index] != Z_NUMBER) {
        // Increase the letter by at least one
        next[index] += 1 + static_cast<u32>(is_invalid_number(next[index]));
        // Go check password validity
        break;
      }
      next[index] = 0;
      if (index > 0) {
        // Go try increasing the letter before this one
        continue;
      }
      // Set to first valid password, aabcc
      AOC_ASSERT(size >= 5,
                 "Entire password is not valid, should have at least size 5");
      next[size - 1] = 2;
      next[size - 2] = 2;
      next[size - 3] = 1;
      AOC_ASSERT(next[size - 4] == 0, "Loop didn't set values properly");
      AOC_ASSERT(next[size - 5] == 0, "Loop didn't set values properly");
      break;
    }
  };
  let modified = remove_initial_invalid(password);
  if (!modified && is_valid_password(password)) {
    increase_by_one(password);
  }
  while (!is_valid_password(password)) {
    increase_by_one(password);
  }
  return password;
}

String next_password_str(str const& password) {
  return to_string(next_password(to_numbers(password)));
}

String solve_case(const String& filename) {
  std::ifstream file{filename};
  return next_password_str(aoc::read_line(file));
}

int main() {
  std::println("Part 1");
  const auto example_solved = solve_case("day11.example");
  AOC_EXPECT_RESULT("ghjaabcc", example_solved);
  const auto input_solved = solve_case("day11.input");
  AOC_EXPECT_RESULT("hxbxxyzz", input_solved);

  std::println("Part 2");
  AOC_EXPECT_RESULT("ghjbbcdd", next_password_str(example_solved));
  AOC_EXPECT_RESULT("hxcaabcc", next_password_str(input_solved));
  AOC_RETURN_CHECK_RESULT();
}
