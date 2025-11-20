// https://adventofcode.com/2023/day/19

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using namespace std::string_view_literals;
using int_t = int;
constexpr inline auto start = std::string{"in"};

struct part_t {
  int x;
  int m;
  int a;
  int s;

  constexpr int sum() const { return x + m + a + s; }

  friend std::ostream& operator<<(std::ostream& out, const part_t& part) {
    out << "{" << part.x << ", " << part.m << ", " << part.a << ", " << part.s
        << "}";
    return out;
  }
};

using rule_t = std::function<std::optional<std::string>(const part_t& part)>;
using workflow_t = std::vector<rule_t>;
using workflow_map_t = std::map<std::string, workflow_t>;

bool check_part(const part_t& part, const workflow_map_t& workflows) {
  auto current = start;
  while (true) {
    const auto& workflow = workflows.at(current);
    for (const auto& rule : workflow) {
      const auto result = rule(part);
      if (!result) {
        continue;
      }
      if (*result == "A") {
        return true;
      }
      if (*result == "R") {
        return false;
      }
      current = *result;
      break;
    }
  }
  return false;
}

int_t sort_parts(const std::vector<part_t>& parts,
                 const workflow_map_t& workflows) {
  return aoc::ranges::accumulate(
      parts | std::views::filter([&](const part_t& part) {
        return check_part(part, workflows);
      }) | std::views::transform(&part_t::sum),
      int_t{0});
}

template <bool>
int_t solve_case(const std::string& filename) {
  std::cout << filename << std::endl;

  std::vector<part_t> parts;
  workflow_map_t workflows;

  const auto parse_rule = [&](std::string_view rule_str) -> rule_t {
    // a<2006:qkq
    // rfg
    if ((rule_str.size() == 1) ||
        ((rule_str[1] != '<') && (rule_str[1] != '>'))) {
      return [return_str = std::string{rule_str}](
                 const part_t&) -> std::optional<std::string> {
        return return_str;
      };
    }
    auto member_ptr = std::mem_fn([&]() {
      switch (rule_str[0]) {
        case 'x':
          return &part_t::x;
        case 'm':
          return &part_t::m;
        case 'a':
          return &part_t::a;
        case 's':
          return &part_t::s;
        default:
          AOC_UNREACHABLE("Invalid part member");
          throw 0;
      }
    }());
    const bool less_than = (rule_str[1] == '<');
    auto [number, rule_rest_str] =
        aoc::to_number_with_rest<int>(rule_str.substr(2));
    return [member_ptr, less_than, number,
            return_str = std::string{rule_rest_str.substr(1)}](
               const part_t& part) -> std::optional<std::string> {
      if (less_than) {
        if (member_ptr(part) < number) {
          return return_str;
        }
      } else {
        if (member_ptr(part) > number) {
          return return_str;
        }
      }
      return std::nullopt;
    };
  };
  const auto parse_workflows = [&](std::string_view line) {
    // px{a<2006:qkq,m>2090:A,rfg}
    auto [name, rules_str_raw] = aoc::split_once(line, '{');
    auto rules_str = aoc::split_to_vec(
        rules_str_raw.substr(0, rules_str_raw.size() - 1), ',');
    workflow_t workflow;
    for (auto rule_str : rules_str) {
      workflow.push_back(parse_rule(rule_str));
    }
    workflows[std::string{name}] = workflow;
  };
  const auto parse_parts = [&](std::string_view line) {
    // {x=787,m=2655,a=1222,s=2876}
    auto [x, m, a, s] =
        aoc::split_to_array<4>(line.substr(1, line.size() - 2), ',');
    part_t part{
        .x = aoc::to_number<int>(x.substr(2)),
        .m = aoc::to_number<int>(m.substr(2)),
        .a = aoc::to_number<int>(a.substr(2)),
        .s = aoc::to_number<int>(s.substr(2)),
    };
    parts.push_back(part);
  };

  bool parsing_parts = false;
  for (std::string_view line :
       aoc::views::read_lines(filename, aoc::keep_empty{})) {
    if (line.empty()) {
      parsing_parts = true;
      continue;
    }
    if (!parsing_parts) {
      parse_workflows(line);
    } else {
      parse_parts(line);
    }
  }

  auto sum = sort_parts(parts, workflows);
  std::cout << "  -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(19114, (solve_case<false>("day19.example")));
  AOC_EXPECT_RESULT(509597, (solve_case<false>("day19.input")));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(952408144115, (solve_case<true>("day19.example")));
  // AOC_EXPECT_RESULT(90111113594927, (solve_case<true>("day19.input")));
  AOC_RETURN_CHECK_RESULT();
}
