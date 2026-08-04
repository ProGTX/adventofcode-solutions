#ifndef AOC_HASH_H
#define AOC_HASH_H

#include "compiler.h"
#include "concepts.h"

#ifndef AOC_MODULE_SUPPORT
#include <algorithm>
#include <ankerl/unordered_dense.h>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <ranges>
#include <string_view>
#include <type_traits>
#endif

AOC_EXPORT_NAMESPACE(aoc) {

/**
 * Fast associative containers.
 *
 * ankerl::unordered_dense stores values densely in a vector
 * and keeps a separate bucket array of indices,
 * so iteration is a linear scan instead of a pointer chase,
 * and a lookup costs no allocation -
 * where a std::unordered_map bucket is a node list, one allocation per element.
 * That is worth several seconds across the search-heavy days,
 * most of which spend their time doing nothing but map lookups.
 *
 * @note Unlike std::unordered_map,
 * insert invalidates iterators and references whenever the value vector grows.
 * 
 * https://martin.ankerl.com/2022/08/27/hashmap-bench-01/
 */
template <class Key, class T, class Hash = ankerl::unordered_dense::hash<Key>,
          class KeyEq = std::equal_to<Key>>
using hash_map = ankerl::unordered_dense::map<Key, T, Hash, KeyEq>;

/// Set counterpart of hash_map.
template <class Key, class Hash = ankerl::unordered_dense::hash<Key>,
          class KeyEq = std::equal_to<Key>>
using hash_set = ankerl::unordered_dense::set<Key, Hash, KeyEq>;

/**
 * Both hashes below are wyhash,
 * reached through unordered_dense's public specializations:
 * hash<string_view> is its byte entry point and hash<uint64_t> its integer one.
 *
 * Neither can be constant evaluated today -
 * wyhash reads memory through memcpy and multiplies through intrinsics -
 * but everything here is marked constexpr anyway.
 */

/**
 * Hashes a block of bytes.
 *
 * wyhash mixes the length in, so this distinguishes byte sequences
 * that differ only by trailing zeros
 * without the caller feeding the size separately.
 */
[[nodiscard]] constexpr std::size_t hash_bytes(const void* data,
                                               std::size_t size) noexcept {
  return static_cast<std::size_t>(
      ankerl::unordered_dense::hash<std::string_view>{}(
          std::string_view{static_cast<const char*>(data), size}));
}

/**
 * Spreads a value's bits so that every input bit affects the whole word.
 *
 * Needed because bucket selection only looks at part of a hash:
 * a value whose information sits outside that part
 * collapses into a handful of buckets
 * and every lookup degrades to a linear scan.
 */
[[nodiscard]] constexpr std::size_t hash_mix(std::size_t value) noexcept {
  return static_cast<std::size_t>(
      ankerl::unordered_dense::hash<std::uint64_t>{}(value));
}

/**
 * Folds several values into one hash, in the order they are fed.
 *
 * The result is read off `seed`:
 * @code
 * auto combine = aoc::hash_combine{};
 * combine(node.pos);
 * combine(node.consecutive);
 * return combine.seed;
 * @endcode
 *
 * https://www.boost.org/doc/libs/1_86_0/libs/container_hash/doc/html/hash.html#notes_hash_combine
 */
struct hash_combine {
  // Non-zero so hashing nothing differs from hashing a zero.
  // 2^31-1, the largest prime that fits a signed 32 bit int.
  std::size_t seed{2'147'483'647};

  AOC_NO_SANITIZE_WRAPAROUND
  constexpr void operator()(std::size_t value) {
    // 0x9e3779b9 is 2^32 divided by golden ratio
    seed ^= hash_mix(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }

  template <std::ranges::input_range R>
    requires(!hashable<R>)
  constexpr void operator()(R&& range) {
    for (auto&& value : range) {
      this->operator()(value);
    }
  }

  template <hashable T>
  constexpr void operator()(const T& value) {
    // unordered_dense's hash falls back to std::hash for its own types,
    // while hashing the built-ins and strings with wyhash directly
    return this->operator()(ankerl::unordered_dense::hash<T>{}(value));
  }
};

/**
 * Whether hash_combine can consume a value of this type.
 *
 * The range overload above only asks that the argument be a range,
 * and a body is not part of a signature,
 * so `requires { combine(value); }` would accept a range
 * whose elements have no hash and only fail once instantiated.
 * One level in covers the shapes used here -
 * deeper nesting reports false and falls back to an ordered container
 * insted of failing to compile.
 */
template <class T>
concept hash_combinable =
    std::convertible_to<T, std::size_t> ||
    hashable<T> ||
    (std::ranges::input_range<T> &&
     (std::convertible_to<std::ranges::range_reference_t<T>, std::size_t> ||
      hashable<std::ranges::range_value_t<T>>));

/**
 * Hashes a value over its whole object representation,
 * for types that are just packed bytes with no meaningful sub-structure.
 *
 * Meant to be inherited into a std::hash specialization:
 * @code
 * template <>
 * struct std::hash<MyStruct> : aoc::packed_hash {};
 * @endcode
 *
 * has_unique_object_representations rules out padding bits,
 * which would otherwise let two equal values carry different garbage
 * and hash differently.
 */
struct packed_hash {
  // Both paths below end in a full wyhash round,
  // so unordered_dense can be told to skip the mixing step
  // it would otherwise add on top
  using is_avalanching = void;

  template <class T>
    requires std::has_unique_object_representations_v<T>
  constexpr std::size_t operator()(const T& value) const noexcept {
    // Anything that fits a word goes to wyhash's integer entry point,
    // which is a straight mix,
    // where its byte entry point first branches on the length.
    // Worth splitting - a two component point takes this path on every lookup.
    if constexpr (sizeof(T) == sizeof(std::uint64_t)) {
      return hash_mix(std::bit_cast<std::uint64_t>(value));
    } else if constexpr (sizeof(T) < sizeof(std::uint64_t)) {
      // bit_cast needs both sides the same size,
      // so the value is zero padded out to a word first.
      // Copied wholesale rather than assembled a byte at a time,
      // which cost more than the hash for a three component point.
      auto padded = std::array<std::byte, sizeof(std::uint64_t)>{};
      const auto bytes = std::bit_cast<std::array<std::byte, sizeof(T)>>(value);
      std::ranges::copy(bytes, padded.begin());
      return hash_mix(std::bit_cast<std::uint64_t>(padded));
    } else {
      return hash_bytes(&value, sizeof(T));
    }
  }
};

} // AOC_EXPORT_NAMESPACE(aoc)

#endif // AOC_HASH_H
