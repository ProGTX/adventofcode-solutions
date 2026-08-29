// The assembunny language, shared between days 12 and 23

#ifndef AOC_2016_ASSEMBUNNY_H
#define AOC_2016_ASSEMBUNNY_H

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <array>
#include <variant>
#endif

namespace assembunny {

using Scalar = i32;
using Registers = std::array<i64, 4>;

/// Registers a to d, as ids 0 to 3
struct Register {
  usize id;
};

/// An instruction's argument: either a literal, or the register holding it
using Value = std::variant<Scalar, Register>;

namespace op {
struct Copy {
  Value from;
  usize to;
};
struct Increase {
  usize id;
};
struct Decrease {
  usize id;
};
struct JumpNotZero {
  Value condition;
  Value offset;
};
/// Days that don't toggle never read the target
struct Toggle {
  usize id;
};
} // namespace op

using Op = std::variant<op::Copy, op::Increase, op::Decrease, op::JumpNotZero,
                        op::Toggle>;

fn parse_register(str word) -> usize {
  AOC_ASSERT((word.size() == 1), "Invalid register");
  return static_cast<usize>(word[0] - 'a');
}

fn parse_value(str word) -> Value {
  if ((word.size() == 1) && (word[0] >= 'a') && (word[0] <= 'd')) {
    return Register{parse_register(word)};
  }
  return aoc::to_number<Scalar>(word);
}

inline auto parse(String const& filename) -> Vec<Op> {
  return aoc::views::read_lines(filename) |
         stdv::transform([](str line) -> Op {
           let words = aoc::split_to_vec<str>(line, ' ');
           if (words[0] == "cpy") {
             return op::Copy{parse_value(words[1]), parse_register(words[2])};
           }
           if (words[0] == "inc") {
             return op::Increase{parse_register(words[1])};
           }
           if (words[0] == "dec") {
             return op::Decrease{parse_register(words[1])};
           }
           if (words[0] == "jnz") {
             return op::JumpNotZero{parse_value(words[1]),
                                    parse_value(words[2])};
           }
           AOC_ASSERT((words[0] == "tgl"), "Invalid instruction");
           return op::Toggle{parse_register(words[1])};
         }) |
         aoc::collect_vec<Op>();
}

fn read(Registers const& registers, Value value) -> i64 {
  return aoc::match(
      value, //
      [](Scalar literal) { return static_cast<i64>(literal); },
      [&](Register reg) { return registers[reg.id]; });
}

} // namespace assembunny

#endif // AOC_2016_ASSEMBUNNY_H
