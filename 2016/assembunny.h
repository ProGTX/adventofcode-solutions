// The assembunny language, shared between days 12 and 23

#ifndef AOC_2016_ASSEMBUNNY_H
#define AOC_2016_ASSEMBUNNY_H

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <array>
#include <cstdlib>
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

/// Transformed instructions
namespace xop {
struct NoOp {};
struct Zero {
  usize id;
};
struct ZeroToggled {
  usize id;
};
struct Add {
  usize id;
  Scalar value;
};
struct LoopAdd {
  usize out_id;
  usize loop_reg;
  Value steps;
};
struct Copy {
  Value from;
  Value to;
};
struct JumpNotZero {
  Value condition;
  Value offset;
};
struct Toggle {
  usize id;
};
} // namespace xop
using XOp =
    std::variant<xop::NoOp, xop::Zero, xop::ZeroToggled, xop::Add, xop::LoopAdd,
                 xop::Copy, xop::JumpNotZero, xop::Toggle>;

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

fn parse(String const& filename) -> Vec<Op> {
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

/// A backwards jump on a register: the end of a loop,
/// with the register it counts on and how far back it goes
struct BackwardsJump {
  usize index;
  usize loop_reg;
  Scalar offset;
};

fn find_backwards_jump(Vec<XOp> const& ops, usize search_from)
    -> Option<BackwardsJump> {
  for (let index : Range{search_from, ops.size()}) {
    let* jump = std::get_if<xop::JumpNotZero>(&ops[index]);
    if (jump == nullptr) {
      continue;
    }
    let* condition = std::get_if<Register>(&jump->condition);
    let* offset = std::get_if<Scalar>(&jump->offset);
    if ((condition != nullptr) && (offset != nullptr) && (*offset < 0)) {
      return BackwardsJump{index, condition->id, *offset};
    }
  }
  return None;
}

fn transform(Vec<Op> const& ops_in) -> Vec<XOp> {
  // Transform individual instructions
  auto ops =
      ops_in |
      stdv::transform([](Op const& op) -> XOp {
        return aoc::match(
            op, //
            [](op::Copy const& copy) -> XOp {
              let* literal = std::get_if<Scalar>(&copy.from);
              if ((literal != nullptr) && (*literal == 0)) {
                return xop::Zero{copy.to};
              }
              return xop::Copy{copy.from, Register{copy.to}};
            },
            [](op::Increase const& inc) -> XOp { return xop::Add{inc.id, 1}; },
            [](op::Decrease const& dec) -> XOp { return xop::Add{dec.id, -1}; },
            [](op::JumpNotZero const& jump) -> XOp {
              return xop::JumpNotZero{jump.condition, jump.offset};
            },
            [](op::Toggle const& toggle) -> XOp {
              return xop::Toggle{toggle.id};
            });
      }) |
      aoc::collect_vec<XOp>();

  // Transform loops
  auto search_from = usize{};
  while (let jump = find_backwards_jump(ops, search_from)) {
    let[index, loop_reg, loop_offset] = *jump;
    search_from = index + 1;

    // offset is negative here
    let start = index - static_cast<usize>(-loop_offset);
    let body = aoc::views::slice(ops, start, index);

    if (!stdr::all_of(body, [](XOp const& op) {
          return std::holds_alternative<xop::Add>(op);
        })) {
      // Only adds are supported in a loop
      continue;
    }

    // If we're incrementing or decrementing register id by 1
    // within the last offset instructions, we know this is a loop
    // NOTE: This assumes that the value of the register
    // is opposite of the sign of the increment/decrement
    let is_loop_counter = [=](usize id, Scalar value) {
      return (id == loop_reg) && (std::abs(value) == 1);
    };
    let is_loop = stdr::any_of(body, [&](XOp const& op) {
      let& add = std::get<xop::Add>(op);
      return is_loop_counter(add.id, add.value);
    });
    if (!is_loop) {
      continue;
    }

    // Modify instructions
    auto new_ops = Vec<XOp>{};
    new_ops.reserve(index + 1 - start);
    for (let& op : body) {
      let& add = std::get<xop::Add>(op);
      if (is_loop_counter(add.id, add.value)) {
        // Zero out loop counter as part of LoopAdd
        continue;
      }
      new_ops.push_back(xop::LoopAdd{add.id, loop_reg, add.value});
    }
    // Keep the number of instructions the same,
    // the offsets held in registers depend on it
    new_ops.push_back(xop::NoOp{}); // Replaces loop counter
    new_ops.push_back(xop::NoOp{}); // Replaces jump op

    stdr::copy(new_ops, ops.begin() + static_cast<isize>(start));
  }

  // Transform a very specific loop
  constexpr let SPECIFIC_LOOP_OFFSET = Scalar{-5};
  for (let index : aoc::views::indices_of(ops)) {
    let* jump = std::get_if<xop::JumpNotZero>(&ops[index]);
    if (jump == nullptr) {
      continue;
    }
    let* outer_reg = std::get_if<Register>(&jump->condition);
    let* offset = std::get_if<Scalar>(&jump->offset);
    if ((outer_reg == nullptr) ||
        (offset == nullptr) ||
        (*offset != SPECIFIC_LOOP_OFFSET)) {
      continue;
    }
    let outer_id = outer_reg->id;
    let start = index - static_cast<usize>(-SPECIFIC_LOOP_OFFSET);

    // The source may be a literal or a register
    let* copy = std::get_if<xop::Copy>(&ops[start]);
    AOC_ASSERT(copy != nullptr, "Unexpected input");
    let* copy_to = std::get_if<Register>(&copy->to);
    AOC_ASSERT(copy_to != nullptr, "Unexpected input");
    let& loop_add = std::get<xop::LoopAdd>(ops[start + 1]);
    let* steps = std::get_if<Scalar>(&loop_add.steps);
    AOC_ASSERT((steps != nullptr) && (*steps == 1), "Unexpected input");
    let out_id = loop_add.out_id;
    let inner_reg = loop_add.loop_reg;
    AOC_ASSERT((copy_to->id == inner_reg),
               "The copy doesn't set the loop register");
    AOC_ASSERT(std::holds_alternative<xop::NoOp>(ops[start + 2]),
               "Expected NoOp");
    AOC_ASSERT(std::holds_alternative<xop::NoOp>(ops[start + 3]),
               "Expected NoOp");
    let& add = std::get<xop::Add>(ops[start + 4]);
    AOC_ASSERT((add.id == outer_id), "Loop register doesn't match");
    AOC_ASSERT((std::abs(add.value) == 1), "Loop increment not 1");

    // ops[start]: Keep the copy as-is
    ops[start + 1] = xop::LoopAdd{out_id, inner_reg, Register{outer_id}};
    // Keep the number of instructions the same,
    // the offsets held in registers depend on it
    ops[start + 2] = xop::NoOp{};
    ops[start + 3] = xop::NoOp{};
    ops[start + 4] = xop::NoOp{};
    ops[index] = xop::NoOp{};
    break;
  }

  return ops;
}

/// Runs a single instruction, moving the counter along with it
fn exec(XOp const& op, Registers& registers, i64& counter) {
  aoc::match(
      op, //
      [&](xop::Zero const& zero) { registers[zero.id] = 0; },
      [&](xop::Add const& add) { registers[add.id] += add.value; },
      [&](xop::LoopAdd const& loop_add) {
        // Fused multiply-add
        registers[loop_add.out_id] +=
            registers[loop_add.loop_reg] * read(registers, loop_add.steps);
        // Loop counter is zero at the end
        registers[loop_add.loop_reg] = 0;
      },
      [&](xop::Copy const& copy) {
        if (let* to = std::get_if<Register>(&copy.to)) {
          registers[to->id] = read(registers, copy.from);
        }
      },
      [&](xop::JumpNotZero const& jump) {
        if (read(registers, jump.condition) != 0) {
          // The jump is relative to this instruction
          counter += read(registers, jump.offset) - 1;
        }
      },
      [](auto const&) {
        // Invalid instruction, do nothing
      });
  counter += 1;
}

} // namespace assembunny

#endif // AOC_2016_ASSEMBUNNY_H
