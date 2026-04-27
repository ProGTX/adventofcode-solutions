// https://adventofcode.com/2021/day/13

#include "../common/common.h"
#include "../common/rust.h"

#include <iostream>
#include <print>
#include <ranges>
#include <string>
#include <unordered_set>

using Point = aoc::point_type<usize>;

using Dots = std::unordered_set<Point>;
using Folds = Vec<Point>;
using Input = std::pair<Dots, Folds>;

auto parse(String const& filename) -> Input {
  auto dots = Dots{};
  auto folds = Folds{};
  auto parsing_folds = false;
  for (str line : aoc::views::read_lines(filename, aoc::keep_empty{})) {
    if (line.empty()) {
      parsing_folds = true;
    } else if (parsing_folds) {
      let rest = line.substr(str{"fold along "}.size());
      let[axis, value_str] = aoc::split_once(rest, '=');
      let n = aoc::to_number<usize>(value_str);
      folds.push_back(axis == "x" ? Point{n, 0} : Point{0, n});
    } else {
      dots.insert(aoc::split<Point>(line, ","));
    }
  }
  return {std::move(dots), std::move(folds)};
}

fn fold_once(Dots const& dots, Point fold) -> Dots {
  auto new_dots = Dots{};
  for (let& dot : dots) {
    if (fold.x == 0) {
      if (dot.y < fold.y) {
        new_dots.insert(dot);
      } else {
        new_dots.insert(Point{dot.x, fold.y - (dot.y - fold.y)});
      }
    } else {
      if (dot.x < fold.x) {
        new_dots.insert(dot);
      } else {
        new_dots.insert(Point{fold.x - (dot.x - fold.x), dot.y});
      }
    }
  }
  return new_dots;
}

fn solve_case1(Input const& input) -> usize {
  let & [ dots, folds ] = input;
  return fold_once(dots, folds[0]).size();
}

fn dots_to_string(Dots const& dots) -> String {
  auto max_x = usize{0};
  auto max_y = usize{0};
  for (let& p : dots) {
    max_x = std::max(max_x, p.x);
    max_y = std::max(max_y, p.y);
  }
  auto rows = Vec<String>{};
  for (usize y = 0; y <= max_y; ++y) {
    auto row = String{};
    for (usize x = 0; x <= max_x; ++x) {
      row += dots.contains(Point{x, y}) ? '#' : '.';
    }
    rows.push_back(std::move(row));
  }
  return aoc::ranges::join(rows, '\n');
}

fn solve_case2(Input const& input) -> Dots {
  let & [ dots_ref, folds ] = input;
  auto dots = dots_ref;
  for (let& fold : folds) {
    dots = fold_once(dots, fold);
  }
  std::println("{}", dots_to_string(dots));
  return dots;
}

int main() {
  std::println("Part 1");
  let example = parse("day13.example");
  AOC_EXPECT_RESULT(17, solve_case1(example));
  let input = parse("day13.input");
  AOC_EXPECT_RESULT(785, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT("#####\n"
                    "#...#\n"
                    "#...#\n"
                    "#...#\n"
                    "#####",
                    dots_to_string(solve_case2(example)));
  AOC_EXPECT_RESULT("####...##..##..#..#...##..##...##..#..#\n"
                    "#.......#.#..#.#..#....#.#..#.#..#.#..#\n"
                    "###.....#.#..#.####....#.#....#..#.####\n"
                    "#.......#.####.#..#....#.#.##.####.#..#\n"
                    "#....#..#.#..#.#..#.#..#.#..#.#..#.#..#\n"
                    "#.....##..#..#.#..#..##...###.#..#.#..#",
                    dots_to_string(solve_case2(input)));

  AOC_RETURN_CHECK_RESULT();
}
