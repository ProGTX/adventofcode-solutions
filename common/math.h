#ifndef AOC_MATH_H
#define AOC_MATH_H

#include "assert.h"

#include <bit>
#include <cmath>
#include <concepts>
#include <type_traits>

namespace aoc {

template <class T>
constexpr T abs(T value) {
  if constexpr (not_constant_before<23>()) {
    return (value < 0) ? -value : value;
  } else if constexpr (std::is_integral_v<T>) {
    return std::abs(value);
  } else {
    return (value < 0) ? -value : value;
  }
}

template <std::integral T>
constexpr T num_digits(T n) {
  return abs(static_cast<std::make_signed_t<T>>(n)) >= 10
             ? num_digits(n / 10) + 1
             : 1;
}

template <std::integral T>
constexpr T sign(T value) {
  return (value < 0) ? -1 : 1;
}
template <std::floating_point T>
constexpr T sign(T value) {
  if constexpr (not_constant_before<23>()) {
    return (value < T{0}) ? T{-1} : T{1};
  } else {
    return std::copysign(T{1.0}, value);
  }
}

// https://stackoverflow.com/a/59420788/793006
template <typename T>
constexpr T pown(T x, unsigned p) {
  T result = 1;

  while (p > 0) {
    if (p & 0x1) {
      result *= x;
    }
    x *= x;
    p >>= 1;
  }

  return result;
}

static_assert(1024 == pown(2, 10));
static_assert(100 == pown(10, 2));

template <std::integral auto Base, std::unsigned_integral T>
constexpr T next_power_of(const T& x) {
  static_assert(Base > 1);
  if constexpr (Base == 2) {
    return std::bit_ceil(x);
  } else {
    T result = 1;
    while (result < x) {
      result *= Base;
    }
    return result;
  }
}

static_assert(8 == next_power_of<2>(7u));
static_assert(100 == next_power_of<10>(16u));

constexpr bool is_number(char c) { return (c >= '0') && (c <= '9'); }

constexpr unsigned flip_bit(unsigned number, unsigned index) {
  const unsigned mask = 1 << index;
  return number ^ mask;
}
static_assert(3 == flip_bit(2, 0));
static_assert(2 == flip_bit(3, 0));

} // namespace aoc

#endif // AOC_MATH_H
