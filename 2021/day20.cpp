// https://adventofcode.com/2021/day/20

#include "../common/common.h"
#include "../common/rust.h"

#include <array>
#include <numeric>
#include <print>
#include <span>
#include <string>
#include <vector>

using Algorithm = Vec<u8>;
using Image = aoc::grid<u8>;
using Input = std::pair<Algorithm, Image>;

static constexpr usize PADDING = 3;

fn to_bit(char c) -> u8 { return static_cast<u8>(c == '#'); }

fn parse(String const& filename) -> Input {
  let lines =
      aoc::views::read_lines(filename) | aoc::collect_vec<std::string>();
  auto algorithm =
      lines[0] | stdv::transform(to_bit) | aoc::ranges::to<Algorithm>();
  constexpr let skip_lines = 1; // read_lines already skips empty lines
  let num_rows = lines.size() - skip_lines;
  let num_cols = lines[skip_lines].size();
  auto image = Image{static_cast<u8>(0u), num_rows + 2 * PADDING,
                     num_cols + 2 * PADDING};
  for (usize row = 0; row < num_rows; ++row) {
    for (usize col = 0; col < num_cols; ++col) {
      image.modify(to_bit(lines[row + skip_lines][col]), row + PADDING,
                   col + PADDING);
    }
  }
  return {std::move(algorithm), std::move(image)};
}

fn apply(Algorithm const& algorithm, Image const& input_image) -> Image {
  let background = static_cast<u8>((algorithm[0] + input_image.at(0, 0)) % 2);
  auto output_image = Image{
      background,
      input_image.num_rows() + 2 * PADDING,
      input_image.num_columns() + 2 * PADDING,
  };
  let num_rows = static_cast<isize>(input_image.num_rows());
  let num_cols = static_cast<isize>(input_image.num_columns());
  for (isize y = 1; y < num_rows - 1; ++y) {
    for (isize x = 1; x < num_cols - 1; ++x) {
      // The index is shifted together directly rather than collected into an
      // array and run through a reversed binary_to_number.
      // Visiting order is top-left first, which ends up as the high bit,
      // matching what the reversed range did.
      auto alg_index = 0uz;
      for (isize dy = -1; dy <= 1; ++dy) {
        for (isize dx = -1; dx <= 1; ++dx) {
          let pixel = input_image.at(static_cast<usize>(y + dy),
                                     static_cast<usize>(x + dx));
          alg_index = (alg_index << 1) | static_cast<usize>(pixel == 1);
        }
      }
      output_image.modify(algorithm[alg_index], static_cast<usize>(y) + PADDING,
                          static_cast<usize>(x) + PADDING);
    }
  }
  return output_image;
}

template <usize ITERATIONS>
fn solve_case(Input const& input) -> u32 {
  let & [ algorithm, input_image ] = input;
  auto image = input_image;
  for (usize i = 0; i < ITERATIONS; ++i) {
    image = apply(algorithm, image);
  }
  return aoc::ranges::accumulate(image, 0u);
}

int main() {
  std::println("Part 1");
  let example = parse("day20.example");
  AOC_EXPECT_RESULT(35, solve_case<2>(example));
  let input = parse("day20.input");
  AOC_EXPECT_RESULT(5249, solve_case<2>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(3351, solve_case<50>(example));
  AOC_EXPECT_RESULT(15714, solve_case<50>(input));

  AOC_RETURN_CHECK_RESULT();
}
