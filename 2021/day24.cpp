// https://adventofcode.com/2021/day/24

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <array>
#include <future>
#include <print>
#include <span>
#include <string>
#include <thread>
#include <variant>
#include <vector>

enum class Reg : usize {
  W = 0,
  X = 1,
  Y = 2,
  Z = 3,
};
constexpr let z_id = static_cast<usize>(Reg::Z);

using Operand = std::variant<Reg, i64>;

enum class Op {
  Inp,
  Add,
  Mul,
  Div,
  Mod,
  Eql,
};

struct Instr {
  Op op;
  Reg dst;
  Operand src;
};

fn parse_reg(str s) -> Reg {
  if (s == "w") {
    return Reg::W;
  }
  if (s == "x") {
    return Reg::X;
  }
  if (s == "y") {
    return Reg::Y;
  }
  return Reg::Z;
}

fn parse_operand(str s) -> Operand {
  if (s == "w" || s == "x" || s == "y" || s == "z") {
    return parse_reg(s);
  }
  return aoc::to_number<i64>(s);
}

fn parse_program(std::span<const str> lines) -> Vec<Instr> {
  auto instrs = Vec<Instr>{};
  for (let line : lines) {
    if (line.empty()) {
      continue;
    }
    let parts = aoc::split_to_array<3>(line, ' ');
    let op = [&]() -> Op {
      if (parts[0] == "inp") {
        return Op::Inp;
      }
      if (parts[0] == "add") {
        return Op::Add;
      }
      if (parts[0] == "mul") {
        return Op::Mul;
      }
      if (parts[0] == "div") {
        return Op::Div;
      }
      if (parts[0] == "mod") {
        return Op::Mod;
      }
      return Op::Eql;
    }();
    let dst = parse_reg(parts[1]);
    let src = (op == Op::Inp) ? Operand{i64{0}} : parse_operand(parts[2]);
    instrs.push_back(Instr{op, dst, src});
  }
  return instrs;
}

fn parse(String const& filename) -> Vec<Instr> {
  let strings = aoc::read_lines(filename);
  return parse_program(
      strings |
      stdv::transform([](const String& s) -> str { return s; }) |
      aoc::collect_vec<str>());
}

using Registers = std::array<i64, 4>;

fn execute(Registers regs, std::span<const Instr> instructions,
           std::span<const i64> input) -> Registers {
  auto input_it = input.begin();
  for (let& instr : instructions) {
    let d = static_cast<usize>(instr.dst);
    let src = aoc::match(
        instr.src, //
        [&](Reg r) { return regs[static_cast<usize>(r)]; },
        [&](i64 n) { return n; });
    regs[d] = [&] {
      switch (instr.op) {
        case Op::Inp:
          return *input_it++;
        case Op::Add:
          return regs[d] + src;
        case Op::Mul:
          return regs[d] * src;
        case Op::Div:
          return regs[d] / src;
        case Op::Mod:
          return regs[d] % src;
        case Op::Eql:
          return static_cast<i64>(regs[d] == src);
        default:
          AOC_UNREACHABLE("Invalid instruction");
      }
    }();
  }
  return regs;
}

// Runs one block for BATCH different z values at once.
// Decoding an instruction is by far the dominant cost of the interpreter,
// and this shares it across the lanes.
// The lanes are independent of each other, so they also vectorize.
constexpr let BATCH = 16uz;
using RegisterBatch = std::array<std::array<i64, BATCH>, 4>;

void execute_batch(RegisterBatch& regs, std::span<const Instr> instructions,
                   i64 input) {
  for (let& instr : instructions) {
    auto src = std::array<i64, BATCH>{};
    aoc::match(
        instr.src, //
        [&](Reg r) { src = regs[static_cast<usize>(r)]; },
        [&](i64 n) { src.fill(n); });
    auto& dst = regs[static_cast<usize>(instr.dst)];
    switch (instr.op) {
      case Op::Inp:
        dst.fill(input);
        break;
      case Op::Add:
        aoc::ranges::static_for<0uz, BATCH>(
            [&](auto i) AOC_FORCE_INLINE { dst[i] += src[i]; });
        break;
      case Op::Mul:
        aoc::ranges::static_for<0uz, BATCH>(
            [&](auto i) AOC_FORCE_INLINE { dst[i] *= src[i]; });
        break;
      case Op::Div:
        aoc::ranges::static_for<0uz, BATCH>(
            [&](auto i) AOC_FORCE_INLINE { dst[i] /= src[i]; });
        break;
      case Op::Mod:
        aoc::ranges::static_for<0uz, BATCH>(
            [&](auto i) AOC_FORCE_INLINE { dst[i] %= src[i]; });
        break;
      case Op::Eql:
        aoc::ranges::static_for<0uz, BATCH>([&](auto i) AOC_FORCE_INLINE {
          dst[i] = static_cast<i64>(dst[i] == src[i]);
        });
        break;
    }
  }
}

fn digits_to_number(std::span<const i64> digits) -> u64 {
  return stdr::fold_left(digits, u64{0}, [](u64 acc, i64 digit) {
    return acc * 10 + static_cast<u64>(digit);
  });
}

constexpr let NUM_BLOCKS = 14uz;

// This limit is somewhat arbitrary, it happens to work for my input
// Could be slightly lower, but this is a nice number
constexpr let Z_LIMIT = i64{314159};

/**
 * The lookup cost is what matters here, not the memory.
 * Every one of the ~40M block runs probes a set,
 * but only ~15k values ever get inserted,
 * and most of those lookups find nothing,
 * so a bitmap beats hashing by a wide margin.
 * Block outputs routinely overshoot the range,
 * which contains() reports as absent.
 * Merging is a plain OR over the words,
 * so the per-thread results are cheap to combine.
 *
 * The set costs Z_LIMIT/8 bytes no matter how few values it holds,
 * and these sets are sparse: the busiest block keeps 10k out of 314k.
 */
using ZSet = aoc::bitmap_set<i64, static_cast<usize>(Z_LIMIT)>;
using ZOutputCache = std::array<ZSet, NUM_BLOCKS>;

// Every z value for one block is independent of every other,
// so the range is split across threads.
// Only the blocks themselves have to stay ordered
// since each one consumes the set produced by the block after it.
fn valid_z_inputs(std::span<const Instr> block, ZSet const& valid_output)
    -> ZSet {
  constexpr let max_parallelism = 16uz;
  let num_threads = std::min<usize>(
      max_parallelism, std::max(1u, std::thread::hardware_concurrency()));
  // A chunk covers whole batches, so the threads write disjoint z values
  let per_thread =
      (static_cast<usize>(Z_LIMIT) + num_threads - 1) / num_threads;
  let chunk = static_cast<i64>((per_thread + BATCH - 1) / BATCH * BATCH);

  auto futures = Vec<std::future<ZSet>>{};
  for (i64 z_start = 0; z_start < Z_LIMIT; z_start += chunk) {
    futures.push_back(std::async(std::launch::async, [=, &valid_output] {
      let z_end = std::min(z_start + chunk, Z_LIMIT);
      auto valid_z_input = ZSet{};
      for (let input : Range{i64{1}, i64{10}}) {
        for (i64 z_base = z_start; z_base < z_end;
             z_base += static_cast<i64>(BATCH)) {
          auto regs = RegisterBatch{};
          aoc::ranges::static_for<0uz, BATCH>([&](auto i) AOC_FORCE_INLINE {
            regs[z_id][i] = z_base + static_cast<i64>(i);
          });
          execute_batch(regs, block, input);
          aoc::ranges::static_for<0uz, BATCH>([&](auto i) AOC_FORCE_INLINE {
            let z = z_base + static_cast<i64>(i);
            if ((z < Z_LIMIT) && valid_output.contains(regs[z_id][i])) {
              valid_z_input.insert(z);
            }
          });
        }
      }
      return valid_z_input;
    }));
  }

  auto valid_z_input = ZSet{};
  for (auto& future : futures) {
    valid_z_input.union_with(future.get());
  }
  return valid_z_input;
}

template <bool SMALLEST>
fn solve_case(std::span<const Instr> instructions,
              Option<ZOutputCache>& valid_z_output_cache) -> u64 {
  // Split instructions into blocks at each Inp instruction
  let starts =
      Range{0uz, instructions.size()} |
      stdv::filter([&](usize i) { return instructions[i].op == Op::Inp; }) |
      aoc::collect_vec<usize>();
  auto blocks = Vec<std::span<const Instr>>{};
  for (let i : Range{0uz, starts.size() - 1}) {
    blocks.push_back(
        instructions.subspan(starts[i], starts[i + 1] - starts[i]));
  }
  blocks.push_back(instructions.subspan(starts.back()));

  // The following algorithm works based on this post:
  // https://www.reddit.com/r/adventofcode/comments/rnqabd/comment/hpu9wk3/

  if (!valid_z_output_cache.has_value()) {
    auto vzo = ZOutputCache{};

    // Find valid z outputs for each block
    vzo[NUM_BLOCKS - 1].insert(0);
    for (let block_id : Range{0uz, NUM_BLOCKS} | stdv::reverse) {
      auto valid_z_input = valid_z_inputs(blocks[block_id], vzo[block_id]);
      if (block_id > 0) {
        vzo[block_id - 1] = std::move(valid_z_input);
      }
    }
    valid_z_output_cache = std::move(vzo);
  }
  let& valid_z_output = *valid_z_output_cache;

  // For each block, find the first input that produces a valid z.
  // Carry over the computed z between blocks.
  auto max_input = std::array<i64, NUM_BLOCKS>{};
  auto z = i64{0};
  for (let[block_id, block] : blocks | stdv::enumerate) {
    for (let input : Range{i64{1}, i64{10}} | stdv::transform([](i64 x) {
                       return SMALLEST ? x : 10 - x;
                     })) {
      auto regs = Registers{};
      regs[z_id] = z;
      let after = execute(regs, block, std::span<const i64>{&input, 1});
      if (valid_z_output[block_id].contains(after[z_id])) {
        max_input[block_id] = input;
        z = after[z_id];
        break;
      }
    }
  }

  return digits_to_number(max_input);
}

fn test(Vec<str> const& lines, Vec<i64> const& input) -> u64 {
  let regs = execute(Registers{}, parse_program(lines), input);
  return digits_to_number(regs);
}

int main() {
  // Unit tests
  static_assert(200 == test(
                           {
                               "inp x",
                               "mul x -1",
                           },
                           {-2}));
  static_assert(901 == test(
                           {
                               "inp z",
                               "inp x",
                               "mul z 3",
                               "eql z x",
                           },
                           {3, 9}));
  static_assert(1101 == test(
                            {
                                "inp w",
                                "add z w",
                                "mod z 2",
                                "div w 2",
                                "add y w",
                                "mod y 2",
                                "div w 2",
                                "add x w",
                                "mod x 2",
                                "div w 2",
                                "mod w 2",
                            },
                            {13}));

  auto valid_z_output = Option<ZOutputCache>{};

  std::println("Part 1");

  let input = parse("day24.input");
  AOC_EXPECT_RESULT(99919765949498, solve_case<false>(input, valid_z_output));

  std::println("Part 2");
  AOC_EXPECT_RESULT(24913111616151, solve_case<true>(input, valid_z_output));

  AOC_RETURN_CHECK_RESULT();
}
