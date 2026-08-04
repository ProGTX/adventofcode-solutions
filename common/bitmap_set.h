#ifndef AOC_BITMAP_SET_H
#define AOC_BITMAP_SET_H

#include "assert.h"
#include "compiler.h"

#ifndef AOC_MODULE_SUPPORT
#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <bit>
#include <concepts>
#include <cstdint>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <vector>
#endif
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
template <class T, std::size_t Limit>
  requires std::integral<T> &&
           (!std::same_as<std::remove_cv_t<T>, bool>) &&
           (Limit > 0)
class bitmap_set {
 private:
  using word_type = std::uint64_t;
  static constexpr const std::size_t word_bits = 64;
  static constexpr const std::size_t num_words =
      (Limit + word_bits - 1) / word_bits;

 public:
  using value_type = T;
  static constexpr const std::size_t limit = Limit;

  /// Walks the set bits in increasing order,
  /// consuming the current word one lowest-set-bit at a time.
  class const_iterator {
   public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using reference = T;
    using pointer = void;

    constexpr const_iterator() = default;
    constexpr const_iterator(const word_type* words, std::size_t word_index)
        : m_words(words), m_word_index(word_index) {
      seek_next_word();
    }

    constexpr T operator*() const {
      return static_cast<T>(
          m_word_index * word_bits +
          static_cast<std::size_t>(std::countr_zero(m_remaining)));
    }

    constexpr const_iterator& operator++() {
      // Clearing the lowest set bit leaves the rest of the word to walk
      m_remaining &= (m_remaining - 1);
      if (m_remaining == 0) {
        ++m_word_index;
        seek_next_word();
      }
      return *this;
    }
    constexpr const_iterator operator++(int) {
      auto copy = *this;
      ++(*this);
      return copy;
    }

    constexpr bool operator==(const const_iterator& other) const {
      return (m_word_index == other.m_word_index) &&
             (m_remaining == other.m_remaining);
    }

   private:
    /// Advances to the first word holding a member,
    /// landing on the end state when there is none left
    constexpr void seek_next_word() {
      while ((m_word_index < num_words) && (m_words[m_word_index] == 0)) {
        ++m_word_index;
      }
      m_remaining =
          (m_word_index < num_words) ? m_words[m_word_index] : word_type{0};
    }

    const word_type* m_words = nullptr;
    std::size_t m_word_index = num_words;
    word_type m_remaining = 0;
  };
  using iterator = const_iterator;

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

  constexpr const_iterator begin() const {
    return const_iterator{m_bits.data(), 0};
  }
  constexpr const_iterator end() const {
    return const_iterator{m_bits.data(), num_words};
  }

  /// The number of members, counted on demand rather than tracked,
  /// because insert() is the hot path and this rarely gets called.
  constexpr std::size_t size() const {
    auto count = std::size_t{0};
    for (const auto word : m_bits) {
      count += static_cast<std::size_t>(std::popcount(word));
    }
    return count;
  }
  constexpr bool empty() const {
    return std::ranges::all_of(m_bits,
                               [](word_type word) { return word == 0; });
  }

  /// Removes all values, keeping the allocation.
  constexpr void clear() { std::ranges::fill(m_bits, word_type{0}); }

  /// Folds another set into this one.
  constexpr void union_with(const bitmap_set& other) {
    for (std::size_t i = 0; i < num_words; ++i) {
      m_bits[i] |= other.m_bits[i];
    }
  }

 private:
  /// Negative values wrap around to a huge index,
  /// so a single comparison against Limit covers both ends of the range
  static constexpr std::size_t index_of(value_type value) {
    return static_cast<std::size_t>(value);
  }

  std::vector<word_type> m_bits;
};

} // AOC_EXPORT_NAMESPACE(aoc)

#endif // AOC_BITMAP_SET_H
