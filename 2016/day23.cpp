// https://adventofcode.com/2016/day/23

#include "assembunny.h"

#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <cstdlib>
#include <print>
#include <utility>
#include <variant>
#endif

using assembunny::Register;
using assembunny::Registers;
using assembunny::Scalar;
using assembunny::Value;

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

using Input = Vec<XOp>;

/// A backwards jump on a register: the end of a loop,
/// with the register it counts on and how far back it goes
struct BackwardsJump {
  usize index;
  usize loop_reg;
  Scalar offset;
};

fn find_backwards_jump(Input const& ops, usize search_from)
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

auto parse(String const& filename) -> Input {
  namespace op = assembunny::op;
  // Transform individual instructions
  auto ops =
      assembunny::parse(filename) |
      stdv::transform([](assembunny::Op const& op) -> XOp {
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

    let* copy = std::get_if<xop::Copy>(&ops[start]);
    AOC_ASSERT((copy != nullptr) &&
                   std::holds_alternative<Register>(copy->from) &&
                   std::holds_alternative<Register>(copy->to),
               "Unexpected input");
    let& loop_add = std::get<xop::LoopAdd>(ops[start + 1]);
    let* steps = std::get_if<Scalar>(&loop_add.steps);
    AOC_ASSERT((steps != nullptr) && (*steps == 1), "Unexpected input");
    let out_id = loop_add.out_id;
    let inner_reg = loop_add.loop_reg;
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

fn toggle(XOp const& op) -> XOp {
  return aoc::match(
      op, //
      [](xop::NoOp const&) -> XOp { return xop::NoOp{}; },
      [](xop::Zero const& zero) -> XOp { return xop::ZeroToggled{zero.id}; },
      [](xop::ZeroToggled const& zero) -> XOp { return xop::Zero{zero.id}; },
      // Add replaces Increase and Decrease
      [](xop::Add const& add) -> XOp {
        return xop::Add{add.id, static_cast<Scalar>(-add.value)};
      },
      [](xop::Toggle const& toggle) -> XOp { return xop::Add{toggle.id, 1}; },
      [](xop::JumpNotZero const& jump) -> XOp {
        return xop::Copy{jump.condition, jump.offset};
      },
      [](xop::Copy const& copy) -> XOp {
        return xop::JumpNotZero{copy.from, copy.to};
      },
      [](xop::LoopAdd const&) -> XOp { AOC_UNREACHABLE("Invalid op"); });
}

/// The value in register a once the program halts
template <i64 NUM_EGGS>
fn solve_case(Input const& input) -> i64 {
  auto ops = input;
  auto registers = Registers{NUM_EGGS, 0, 0, 0};
  auto counter = i64{};
  while ((counter >= 0) && (static_cast<usize>(counter) < ops.size())) {
    let op = ops[static_cast<usize>(counter)];
    if (let* toggle_op = std::get_if<xop::Toggle>(&op)) {
      // The target is relative to this instruction,
      // and may be outside the program
      let target = counter + registers[toggle_op->id];
      if ((target >= 0) && (static_cast<usize>(target) < ops.size())) {
        auto& target_op = ops[static_cast<usize>(target)];
        target_op = toggle(target_op);
      }
      counter += 1;
      continue;
    }
    aoc::match(
        op, //
        [&](xop::Zero const& zero) { registers[zero.id] = 0; },
        [&](xop::Add const& add) { registers[add.id] += add.value; },
        [&](xop::LoopAdd const& loop_add) {
          // Fused multiply-add
          registers[loop_add.out_id] +=
              registers[loop_add.loop_reg] *
              assembunny::read(registers, loop_add.steps);
          // Loop counter is zero at the end
          registers[loop_add.loop_reg] = 0;
        },
        [&](xop::Copy const& copy) {
          if (let* to = std::get_if<Register>(&copy.to)) {
            registers[to->id] = assembunny::read(registers, copy.from);
          }
        },
        [&](xop::JumpNotZero const& jump) {
          if (assembunny::read(registers, jump.condition) != 0) {
            // The jump is relative to this instruction
            counter += assembunny::read(registers, jump.offset) - 1;
          }
        },
        [](auto const&) {
          // Invalid instruction, do nothing
        });
    counter += 1;
  }
  // Register a
  return registers[0];
}

int main() {
  std::println("Part 1");
  let example = parse("day23.example");
  AOC_EXPECT_RESULT(3, solve_case<7>(example));
  let input = parse("day23.input");
  AOC_EXPECT_RESULT(11739, solve_case<7>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(3, solve_case<12>(example));
  AOC_EXPECT_RESULT(479008299, solve_case<12>(input));

  AOC_RETURN_CHECK_RESULT();
}
