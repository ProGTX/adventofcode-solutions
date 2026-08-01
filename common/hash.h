#ifndef AOC_HASH_H
#define AOC_HASH_H

#include "compiler.h"
#include "concepts.h"

#ifndef AOC_MODULE_SUPPORT
#include <array>
#include <bit>
#include <cstddef>
#include <functional>
#include <ranges>
#include <type_traits>
#endif

AOC_EXPORT_NAMESPACE(aoc) {

/// https://www.boost.org/doc/libs/1_86_0/libs/container_hash/doc/html/hash.html#notes_hash_combine
struct hash_combine {
  std::size_t seed;

  // Wraparound is intentional here
  [[clang::no_sanitize("unsigned-integer-overflow", "unsigned-shift-base")]]
  constexpr void operator()(std::size_t value) {
    seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
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
    return this->operator()(std::hash<T>{}(value));
  }
};

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
  template <class T>
    requires std::has_unique_object_representations_v<T>
  constexpr std::size_t operator()(const T& value) const noexcept {
    constexpr auto size = sizeof(T);
    constexpr auto chunk = sizeof(std::size_t);
    auto combine = hash_combine{};

    // bit_cast rather than memcpy so this stays usable in constexpr
    // Both sides have to be the exact same size
    if constexpr ((size % chunk) == 0) {
      // Exact fit, so cast straight to size_t words and fold them
      // No repacking, and no partial chunk to pad
      const auto words =
          std::bit_cast<std::array<std::size_t, size / chunk>>(value);
      combine(words);
    } else {
      // Leftover bytes, so the words have to be built up by hand
      const auto bytes = std::bit_cast<std::array<std::byte, size>>(value);

      // Fold a whole size_t per round rather than a byte per round,
      // e.g. a 27 byte type costs 4 rounds instead of 27
      for (std::size_t i = 0; i < size; i += chunk) {
        // Reassemble the word a byte at a time,
        // since bit_cast can't slice the array
        // Also zero pads the last chunk when size isn't an exact multiple,
        // which is what the (i + j) < size guard is for
        auto word = std::array<std::byte, chunk>{};
        for (std::size_t j = 0; (j < chunk) && ((i + j) < size); ++j) {
          word[j] = bytes[i + j];
        }
        // Feeding size_t hits hash_combine's constexpr overload
        // Handing it the byte array instead would pick the range overload
        // and hash each std::byte through std::hash,
        // which isn't guaranteed in current standard libraries
        combine(std::bit_cast<std::size_t>(word));
      }
    }
    return combine.seed;
  }
};

} // AOC_EXPORT_NAMESPACE(aoc)

#endif // AOC_HASH_H
