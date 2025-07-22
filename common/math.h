#ifndef AOC_MATH_H
#define AOC_MATH_H

#include "assert.h"

#ifndef AOC_MODULE_SUPPORT
#include <bit>
#include <cmath>
#include <concepts>
#include <type_traits>
#endif

AOC_EXPORT namespace aoc {

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

// (n, k) = n! / (k! * (n-k)!)
// (n, k) = (n - 1, k - 1) * n / k
// https://en.wikipedia.org/wiki/Binomial_coefficient#Identities_involving_binomial_coefficients
template <std::integral T>
constexpr T binomial_coefficient(const T n, const T k) {
  AOC_ASSERT((n >= k) && (k >= 0), "Invalid starting values");
  if ((k == 0) || (n == k)) {
    return 1;
  }

  auto previous = n - k + 1;
  for (int ki = 1; ki < static_cast<int>(k); ++ki) {
    auto current = previous * (n - k + 1 + ki) / (ki + 1);
    previous = current;
  }
  return previous;
}
static_assert(1 == binomial_coefficient(0, 0));
static_assert(1 == binomial_coefficient(1, 1));
static_assert(1 == binomial_coefficient(1, 0));
static_assert(1 == binomial_coefficient(2, 2));
static_assert(1 == binomial_coefficient(3, 3));
static_assert(4 == binomial_coefficient(4, 3));
static_assert(6 == binomial_coefficient(4, 2));
static_assert(10 == binomial_coefficient(5, 3));
static_assert(924 == binomial_coefficient(12, 6));
static_assert(1716 == binomial_coefficient(13, 7));
static_assert(1716 == binomial_coefficient(13, 6));

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

constexpr std::unsigned_integral auto concat_numbers(
    std::unsigned_integral auto a, std::unsigned_integral auto b) {
  // The plus one is important
  return a * next_power_of<10>(b + 1) + b;
}
static_assert(1019 == concat_numbers(10u, 19u));
static_assert(156 == concat_numbers(15u, 6u));
static_assert(841 == concat_numbers(84u, 1u));
static_assert(56310 == concat_numbers(563u, 10u));

constexpr bool is_number(char c) { return (c >= '0') && (c <= '9'); }

constexpr unsigned flip_bit(unsigned number, unsigned index) {
  const unsigned mask = 1 << index;
  return number ^ mask;
}
static_assert(3 == flip_bit(2, 0));
static_assert(2 == flip_bit(3, 0));

} // namespace aoc

#endif // AOC_MATH_H
