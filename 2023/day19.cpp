// https://adventofcode.com/2023/day/19

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <array>
#include <print>
#include <variant>

constexpr str start_name = "in";

struct part_t {
  std::array<i32, 4> xmas;

  fn sum() const -> i32 { return aoc::ranges::accumulate(xmas, i32{0}); }
};

struct accepted_t {};
struct rejected_t {};
using destination_t = std::variant<accepted_t, rejected_t, usize>;

constexpr let no_comparison = 4uz;

struct rule_t {
  destination_t destination;
  usize member = no_comparison;
  i32 number = 0;
};

using workflow_t = Vec<rule_t>;
using workflows_t = Vec<workflow_t>;

struct input_t {
  Vec<part_t> parts;
  workflows_t workflows;
  usize start;
};

fn rule_matches(part_t const& part, rule_t const& rule) -> bool {
  if (rule.member == no_comparison) {
    return true;
  }
  let value = part.xmas[rule.member];
  return (rule.number > 0) ? (value > rule.number) : (value < -rule.number);
}

fn check_part(part_t const& part, workflows_t const& workflows, usize start)
    -> bool {
  auto current = start;
  while (true) {
    let& workflow = workflows[current];
    for (let& rule : workflow) {
      if (!rule_matches(part, rule)) {
        continue;
      }
      if (std::holds_alternative<accepted_t>(rule.destination)) {
        return true;
      }
      if (std::holds_alternative<rejected_t>(rule.destination)) {
        return false;
      }
      current = std::get<usize>(rule.destination);
      break;
    }
  }
  return false;
}

fn solve_case1(input_t const& input) -> i32 {
  return aoc::ranges::accumulate(
      input.parts | stdv::filter([&](part_t const& part) {
        return check_part(part, input.workflows, input.start);
      }) | stdv::transform(&part_t::sum),
      i32{0});
}

fn parse(String const& filename) -> input_t {
  input_t input;

  auto name_to_id = aoc::name_to_id{};
  input.start = name_to_id.intern(start_name);

  let parse_destination = [&](str dest_str) -> destination_t {
    if (dest_str == "A") {
      return accepted_t{};
    }
    if (dest_str == "R") {
      return rejected_t{};
    }
    return name_to_id.intern(dest_str);
  };
  let parse_rule = [&](str rule_str) -> rule_t {
    // a<2006:qkq
    // rfg
    if ((rule_str.size() == 1) ||
        ((rule_str[1] != '<') && (rule_str[1] != '>'))) {
      return rule_t{.destination = parse_destination(rule_str)};
    }
    let member = [&]() -> usize {
      switch (rule_str[0]) {
        case 'x':
          return 0;
        case 'm':
          return 1;
        case 'a':
          return 2;
        case 's':
          return 3;
        default:
          AOC_UNREACHABLE("Invalid part member");
          throw 0;
      }
    }();
    let less_than = (rule_str[1] == '<');
    let[number, rule_rest_str] =
        aoc::to_number_with_rest<i32>(rule_str.substr(2));
    return rule_t{
        .destination = parse_destination(rule_rest_str.substr(1)),
        .member = member,
        .number = less_than ? -number : number,
    };
  };
  let parse_workflow = [&](str line) {
    // px{a<2006:qkq,m>2090:A,rfg}
    let[name, rules_str_raw] = aoc::split_once(line, '{');
    let rules_str = aoc::split_to_vec(
        rules_str_raw.substr(0, rules_str_raw.size() - 1), ',');
    workflow_t workflow;
    for (let rule_str : rules_str) {
      workflow.push_back(parse_rule(rule_str));
    }
    let id = name_to_id.intern(name);
    input.workflows.resize(name_to_id.new_size(input.workflows.size()));
    input.workflows[id] = std::move(workflow);
  };
  let parse_part = [&](str line) {
    // {x=787,m=2655,a=1222,s=2876}
    let[x, m, a, s] =
        aoc::split_to_array<4>(line.substr(1, line.size() - 2), ',');
    input.parts.push_back(part_t{
        .xmas =
            {
                aoc::to_number<i32>(x.substr(2)),
                aoc::to_number<i32>(m.substr(2)),
                aoc::to_number<i32>(a.substr(2)),
                aoc::to_number<i32>(s.substr(2)),
            },
    });
  };

  bool parsing_parts = false;
  for (str line : aoc::views::read_lines(filename, aoc::keep_empty{})) {
    if (line.empty()) {
      parsing_parts = true;
      continue;
    }
    if (!parsing_parts) {
      parse_workflow(line);
    } else {
      parse_part(line);
    }
  }

  return input;
}

struct range_t {
  i32 start;
  i32 end;

  fn size() const -> i64 { return static_cast<i64>(end - start + 1); }
};

fn range_product(std::array<range_t, 4> const& ranges) -> i64 {
  i64 product = 1;
  for (let& range : ranges) {
    product *= range.size();
  }
  return product;
}

fn count_accepted(workflows_t const& workflows, usize workflow_id,
                  std::array<range_t, 4> ranges) -> i64 {
  auto total = i64{};
  for (let& rule : workflows[workflow_id]) {
    auto matching = ranges;
    bool has_match = true;
    if (rule.member != no_comparison) {
      auto& match_range = matching[rule.member];
      auto& rest_range = ranges[rule.member];
      if (rule.number > 0) {
        match_range.start = std::max(match_range.start, rule.number + 1);
        rest_range.end = std::min(rest_range.end, rule.number);
      } else {
        match_range.end = std::min(match_range.end, -rule.number - 1);
        rest_range.start = std::max(rest_range.start, -rule.number);
      }
      has_match = match_range.start <= match_range.end;
    }

    if (has_match) {
      if (std::holds_alternative<accepted_t>(rule.destination)) {
        total += range_product(matching);
      } else if (std::holds_alternative<usize>(rule.destination)) {
        total += count_accepted(workflows, std::get<usize>(rule.destination),
                                matching);
      }
    }

    if ((rule.member == no_comparison) ||
        (ranges[rule.member].start > ranges[rule.member].end)) {
      break;
    }
  }
  return total;
}

fn solve_case2(input_t const& input) -> i64 {
  constexpr range_t full_range{1, 4000};
  return count_accepted(input.workflows, input.start,
                        {full_range, full_range, full_range, full_range});
}

int main() {
  std::println("Part 1");
  let example = parse("day19.example");
  AOC_EXPECT_RESULT(19114, solve_case1(example));
  let input = parse("day19.input");
  AOC_EXPECT_RESULT(509597, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(167409079868000, solve_case2(example));
  AOC_EXPECT_RESULT(143219569011526, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
