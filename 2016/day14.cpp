// https://adventofcode.com/2016/day/14

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <array>
#include <charconv>
#include <print>
#include <span>
#include <thread>
#endif

using Input = String;

using Hash = std::array<unsigned char, 16>;

/// How far ahead a candidate's confirming run of five is looked for
constexpr u32 LOOKAHEAD = 1000;

constexpr usize NUM_DIGITS = 2 * sizeof(Hash);

/// How many distinct values a hex digit can take
constexpr usize NUM_VALUES = 16;

constexpr str HEX_DIGITS = "0123456789abcdef";

fn parse(String const& filename) -> Input {
  return aoc::read_single_line(filename);
}

fn stretch(Hash const& hash) -> Hash {
  auto buffer = std::array<char, NUM_DIGITS>{};
  // A byte at a time, so each nibble costs a shift and a mask
  // rather than the division `digit` would do
  for (let index : Range{0uz, hash.size()}) {
    let byte = hash[index];
    buffer[2 * index] = HEX_DIGITS[byte >> 4];
    buffer[(2 * index) + 1] = HEX_DIGITS[byte & 0xf];
  }
  return aoc::md5(str{buffer.data(), buffer.size()});
}

template <u32 NumStretches>
fn compute_hash(str salt, u32 index) -> Hash {
  auto buffer = std::array<char, 32>{};
  stdr::copy(salt, std::begin(buffer));
  let[end, _] =
      std::to_chars(std::begin(buffer) + salt.size(), std::end(buffer), index);
  auto hash = aoc::md5(str{std::begin(buffer), end});
  for (let _ : Range{0u, NumStretches}) {
    hash = stretch(hash);
  }
  return hash;
}

/// The hash as it is written out: one hex digit per nibble
fn digit(Hash const& hash, usize index) -> u8 {
  let byte = hash[index / 2];
  return ((index % 2) == 0) ? static_cast<u8>(byte >> 4)
                            : static_cast<u8>(byte & 0xf);
}

/// What a single hash contributes
struct Runs {
  /// The digit of its first run of three
  Option<u8> candidate_digit = None;
  /// Which digits appear in a run of five, indexed by digit
  std::array<bool, NUM_VALUES> confirming_digits{};
};

/// Both answers come out of one pass over the hash's digits
fn find_runs(Hash const& hash) -> Runs {
  auto runs = Runs{};
  auto length = 1uz;
  for (let index : Range{1uz, NUM_DIGITS}) {
    let current = digit(hash, index);
    if (current == digit(hash, index - 1)) {
      ++length;
    } else {
      length = 1;
    }
    if ((length == 3) && !runs.candidate_digit) {
      runs.candidate_digit = current;
    }
    if (length >= 5) {
      runs.confirming_digits[current] = true;
    }
  }
  return runs;
}

fn div_ceil(usize value, usize divisor) -> usize {
  return (value + divisor - 1) / divisor;
}

/// Fills in the runs of the hashes starting at index `first`, one per slot.
/// The indices are independent,
/// so the slots are split across the available threads
template <u32 NumStretches>
fn find_runs_range(str salt, u32 first, std::span<Runs> runs) {
  /// Below this a thread does not earn what it costs to start
  constexpr usize MIN_HASHES_PER_THREAD = 1uz << 12;
  let num_hashes = runs.size() * (1uz + NumStretches);
  let num_threads =
      std::max(1uz, std::min(usize{aoc::num_worker_threads()},
                             div_ceil(num_hashes, MIN_HASHES_PER_THREAD)));
  let chunk = div_ceil(runs.size(), num_threads);
  {
    auto threads = Vec<std::jthread>{};
    threads.reserve(num_threads);
    for (auto offset = 0uz; offset < runs.size(); offset += chunk) {
      let slots = runs.subspan(offset, std::min(chunk, runs.size() - offset));
      let start = first + static_cast<u32>(offset);
      threads.emplace_back([salt, start, slots] {
        for (let index : Range{0uz, slots.size()}) {
          slots[index] = find_runs(compute_hash<NumStretches>(
              salt, start + static_cast<u32>(index)));
        }
      });
    }
  } // all threads join here
}

/// How many keys are needed to fill out the one-time pad
constexpr u32 NUM_KEYS = 64;

template <u32 NumStretches>
fn solve_case(Input const& salt) -> u32 {
  // A candidate needs the LOOKAHEAD hashes after it, so a block of that many
  // can only be checked once a second block is there to confirm against.
  // Both blocks live in this one buffer, which every refill overwrites in place
  auto runs = Vec<Runs>(2 * usize{LOOKAHEAD});
  find_runs_range<NumStretches>(salt, 0, runs);
  // The candidate is the first hash of the window, the rest is what confirms it
  let is_key = [&runs](usize offset) {
    let candidate = runs[offset].candidate_digit;
    if (!candidate) {
      return false;
    }
    return stdr::any_of(std::span{runs}.subspan(offset + 1, usize{LOOKAHEAD}),
                        [wanted = *candidate](Runs const& run) {
                          return run.confirming_digits[wanted];
                        });
  };

  auto num_keys = 0u;
  auto first = 0u;
  loop {
    for (let offset : Range{0u, LOOKAHEAD}) {
      if (is_key(offset)) {
        ++num_keys;
        if (num_keys == NUM_KEYS) {
          return first + offset;
        }
      }
    }

    // Drop the block just checked, and read another one in ahead of the search
    first += LOOKAHEAD;
    stdr::copy(runs | stdv::drop(usize{LOOKAHEAD}), std::begin(runs));
    find_runs_range<NumStretches>(salt, first + LOOKAHEAD,
                                  std::span{runs}.subspan(usize{LOOKAHEAD}));
  }
}

int main() {
  std::println("Part 1");
  let example = parse("day14.example");
  AOC_EXPECT_RESULT(22728, solve_case<0>(example));
  let input = parse("day14.input");
  AOC_EXPECT_RESULT(18626, solve_case<0>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(22551, solve_case<2016>(example));
  AOC_EXPECT_RESULT(20092, solve_case<2016>(input));

  AOC_RETURN_CHECK_RESULT();
}
