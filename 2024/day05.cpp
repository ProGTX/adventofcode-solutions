// https://adventofcode.com/2024/day/5

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iterator>
#include <print>
#include <set>
#include <span>

// The input (pages) is limited to numbers below 100
// For each page we store a list of pages that must come before it and after it
using relation_list_t = aoc::static_vector<i32, 100>;
struct ordering_rule_t {
  relation_list_t before;
  relation_list_t after;
};

// The input is limited to updates of at most 70 pages
using update_t = aoc::static_vector<i32, 70>;

struct Input {
  Vec<ordering_rule_t> rules;
  Vec<update_t> updates;
};

fn parse(String const& filename) -> Input {
  let lines = aoc::views::read_lines(filename, aoc::keep_empty{}) |
              aoc::collect_vec<String>();
  let separator_it = stdr::find(lines, "");

  // We use the vector as a hashmap, so we need to use enough memory
  // for all the pages
  using rules_t = Vec<ordering_rule_t>;
  let rule_lines = std::span{lines.begin(), separator_it};
  auto rules =
      stdr::fold_left(rule_lines, rules_t(100), [](rules_t rules, str line) {
        let[before, after] = aoc::split_once<i32>(line, '|');
        rules[before].after.push_back(after);
        rules[after].before.push_back(before);
        return rules;
      });

  let update_lines = std::span{separator_it + 1, lines.end()};
  auto updates = update_lines |
                 stdv::transform(
                     [](str line) { return aoc::split<update_t>(line, ','); }) |
                 aoc::collect_vec<update_t>();

  return {std::move(rules), std::move(updates)};
}

fn index_of_correct_update(Vec<ordering_rule_t> const& rules,
                           std::span<const i32> update) -> i32 {
  AOC_ASSERT(rules.size() >= 100,
             "Vector of rules used as a hashmap, "
             "so it needs to have enough space to store all pages");
  let is_ordered = stdr::all_of(Range{0uz, update.size()}, [&](usize u) {
    let page = update[u];
    // Check that pages before are not in the rules list of pages after
    let before_ok = stdr::none_of(update.first(u), [&](i32 before) {
      return stdr::contains(rules[page].after, before);
    });
    // Check that pages after are not in the rules list of pages before
    let after_ok = stdr::none_of(update.subspan(u + 1), [&](i32 after) {
      return stdr::contains(rules[page].before, after);
    });
    return before_ok && after_ok;
  });
  return is_ordered ? static_cast<i32>(update.size() / 2) : -1;
}

// Orders the update and returns the middle element
fn order_update(Vec<ordering_rule_t> const& rules, std::span<const i32> update)
    -> i32 {
  AOC_ASSERT(rules.size() >= 100,
             "Vector of rules used as a hashmap, "
             "so it needs to have enough space to store all pages");
  let comparator = [&](i32 lhs, i32 rhs) {
    return stdr::contains(rules[rhs].before, lhs);
  };
  auto ordered_update = aoc::flat_set<i32, decltype(comparator)>{comparator};
  stdr::copy(update, std::inserter(ordered_update, ordered_update.end()));
  auto it = std::begin(ordered_update);
  std::advance(it, ordered_update.size() / 2);
  return *it;
}

template <bool order>
fn solve_case(Input const& input) -> i32 {
  return aoc::ranges::accumulate(
      input.updates | stdv::transform([&](update_t const& update) -> i32 {
        let index = index_of_correct_update(input.rules, update);
        if constexpr (!order) {
          return (index >= 0) ? update[index] : 0;
        } else {
          return (index < 0) ? order_update(input.rules, update) : 0;
        }
      }),
      i32{});
}

int main() {
  std::println("Part 1");
  let example2 = parse("day05.example2");
  AOC_EXPECT_RESULT(61, solve_case<false>(example2));
  let example3 = parse("day05.example3");
  AOC_EXPECT_RESULT(0, solve_case<false>(example3));
  let example = parse("day05.example");
  AOC_EXPECT_RESULT(143, solve_case<false>(example));
  let input = parse("day05.input");
  AOC_EXPECT_RESULT(6034, solve_case<false>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(0, solve_case<true>(example2));
  AOC_EXPECT_RESULT(47, solve_case<true>(example3));
  AOC_EXPECT_RESULT(123, solve_case<true>(example));
  AOC_EXPECT_RESULT(6305, solve_case<true>(input));

  AOC_RETURN_CHECK_RESULT();
}
