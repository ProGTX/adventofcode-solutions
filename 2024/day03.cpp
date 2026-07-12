// https://adventofcode.com/2024/day/3

#include "../common/common.h"
#include "../common/rust.h"

#include <print>

auto parse(String const& filename) -> String {
  return aoc::read_file(filename);
}

constexpr let max_operand_digits = usize{3};

// A cursor over the input,
// in the spirit of the Kaleidoscope tutorial's token stream:
// `try*` methods attempt to consume a semantic element at the current position
// and rewind on failure so the caller can try something else instead
// (or just skip a character).
struct Cursor {
  str bytes;
  usize pos = 0;

  fn peek() const -> Option<char> {
    return (pos < bytes.size()) ? Option<char>{bytes[pos]} : None;
  }

  fn advance() -> Option<char> {
    let byte = peek();
    if (byte) {
      ++pos;
    }
    return byte;
  }

  fn try_consume_literal(str literal) -> bool {
    let start = pos;
    for (char expected : literal) {
      if (advance() != expected) {
        pos = start;
        return false;
      }
    }
    return true;
  }

  fn try_parse_number() -> Option<u32> {
    let start = pos;
    while ((pos - start) < max_operand_digits) {
      let next = peek();
      if (!next || !aoc::is_number(*next)) {
        break;
      }
      (void)advance();
    }
    if (pos == start) {
      return None;
    }
    return aoc::to_number<u32>(bytes.substr(start, pos - start));
  }

  fn try_parse_mul() -> Option<u32> {
    let start = pos;
    if (!try_consume_literal("mul(")) {
      pos = start;
      return None;
    }
    let left = try_parse_number();
    if (!left || !try_consume_literal(",")) {
      pos = start;
      return None;
    }
    let right = try_parse_number();
    if (!right || !try_consume_literal(")")) {
      pos = start;
      return None;
    }
    // Great success
    return *left * *right;
  }
};

fn parse_multiplications(str line) -> u32 {
  auto cursor = Cursor{line};
  auto sum = u32{};
  while (cursor.peek()) {
    if (let product = cursor.try_parse_mul()) {
      sum += *product;
    } else {
      (void)cursor.advance();
    }
  }
  return sum;
}

fn parse_multiplications_with_enablers(str line) -> u32 {
  auto cursor = Cursor{line};
  u32 sum = 0;
  bool mul_enabled = true;
  while (cursor.peek()) {
    if (cursor.try_consume_literal("do()")) {
      mul_enabled = true;
    } else if (cursor.try_consume_literal("don't()")) {
      mul_enabled = false;
    } else if (let product = cursor.try_parse_mul()) {
      sum += *product * static_cast<u32>(mul_enabled);
    } else {
      (void)cursor.advance();
    }
  }
  return sum;
}

static_assert(2024 == parse_multiplications("mul(44,46)"));
static_assert(33 ==
              parse_multiplications("xmul(2,4)&mul[3,7]!^don't()_mul(5,5)"));
static_assert(8 == parse_multiplications_with_enablers(
                       "xmul(2,4)&mul[3,7]!^don't()_mul(5,5)"));
static_assert(1019406 == parse_multiplications(
                             "[#from())when()/}+%mul(982,733)mul(700,428)}}"));

int main() {
  std::println("Part 1");
  let example = parse("day03.example");
  AOC_EXPECT_RESULT(161, parse_multiplications(example));
  let example2 = parse("day03.example2");
  AOC_EXPECT_RESULT(161, parse_multiplications(example2));
  let input = parse("day03.input");
  AOC_EXPECT_RESULT(174960292, parse_multiplications(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(161, parse_multiplications_with_enablers(example));
  AOC_EXPECT_RESULT(48, parse_multiplications_with_enablers(example2));
  AOC_EXPECT_RESULT(56275602, parse_multiplications_with_enablers(input));

  AOC_RETURN_CHECK_RESULT();
}
