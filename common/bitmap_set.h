#ifndef AOC_BITMAP_SET_H
#define AOC_BITMAP_SET_H

#include "assert.h"
#include "compiler.h"

#ifndef AOC_MODULE_SUPPORT
#include <algorithm>
#include <concepts>
#include <cstdint>
#include <ranges>
#include <type_traits>
#include <vector>
#endif

AOC_EXPORT_NAMESPACE(aoc) {

/**
 * A set of integers stored as a bitmap:
 * one bit per value in [0, Limit), packed 64 to a word.
 * The value is its own index, so a lookup is a shift, a mask, and one load,
 * with no hashing and no probing.
 *
 * A bitmap is worth it when the range of possible values
 * is small enough to keep in cache,
 * and when lookups far outnumber insertions.
 * It costs Limit/8 bytes no matter how few values it holds,
 * so it is a poor fit for a sparse set over a wide range.
 *
 * Values outside [0, Limit) are never members:
 * contains() reports them as absent rather than treating them as an error,
 * because producers routinely overshoot the range.
 * insert() does require its argument to be in range.
 */
template <class T, size_t Limit>
  requires std::integral<T> &&
           (!std::same_as<std::remove_cv_t<T>, bool>) &&
           (Limit > 0)
class bitmap_set {
 private:
  using word_type = std::uint64_t;
  static constexpr const size_t word_bits = 64;
  static constexpr const size_t num_words = (Limit + word_bits - 1) / word_bits;

 public:
  using value_type = T;
  static constexpr const size_t limit = Limit;

  /// An empty set covering the whole [0, Limit) range.
  constexpr bitmap_set() : m_bits(num_words, word_type{0}) {}

  /// Whether a value can be a member at all.
  static constexpr bool in_range(value_type value) {
    return index_of(value) < Limit;
  }

  /// Membership test for an arbitrary value.
  /// Out of range values are not members.
  constexpr bool contains(value_type value) const {
    if (!in_range(value)) {
      return false;
    }
    const auto index = index_of(value);
    return ((m_bits[index / word_bits] >> (index % word_bits)) & 1) != 0;
  }

  /// Adds a value, which must be in range.
  /// Returns whether it was not already a member,
  /// so a find/insert pair collapses into a single lookup.
  constexpr bool insert(value_type value) {
    AOC_ASSERT(in_range(value), "Value must be in [0, Limit)");
    const auto index = index_of(value);
    const auto mask = word_type{1} << (index % word_bits);
    auto& word = m_bits[index / word_bits];
    const bool was_member = (word & mask) != 0;
    word |= mask;
    return !was_member;
  }

  /// Removes all values, keeping the allocation.
  constexpr void clear() { std::ranges::fill(m_bits, word_type{0}); }

  /// Folds another set into this one.
  constexpr void union_with(const bitmap_set& other) {
    for (size_t i = 0; i < num_words; ++i) {
      m_bits[i] |= other.m_bits[i];
    }
  }

 private:
  /// Negative values wrap around to a huge index,
  /// so a single comparison against Limit covers both ends of the range
  static constexpr size_t index_of(value_type value) {
    return static_cast<size_t>(value);
  }

  std::vector<word_type> m_bits;
};

} // AOC_EXPORT_NAMESPACE(aoc)

#endif // AOC_BITMAP_SET_H
