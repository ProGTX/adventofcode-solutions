#ifndef AOC_RATIO_H
#define AOC_RATIO_H

#include "math.h"

#include <compare>
#include <concepts>
#include <numeric>
#include <system_error>

// Runtime ratio class to replace std::ratio
// https://en.cppreference.com/w/cpp/header/ratio

namespace aoc {

template <std::integral T>
class ratio {
 private:
  struct no_check {};
  struct no_simplify {};

 public:
  using value_type = T;

  constexpr ratio(value_type numerator_ = 0) noexcept
      : m_numerator(numerator_), m_denominator(1) {}

  constexpr ratio(value_type numerator_, value_type denominator_)
      : m_numerator(aoc::sign(numerator_) * aoc::sign(denominator_) *
                    aoc::abs(numerator_)),
        m_denominator(aoc::abs(denominator_)) {
    if (m_denominator == 0) {
      throw std::runtime_error("Ratio denominator cannot be zero");
    }
    simplify();
  }

  constexpr ratio(const ratio&) = default;
  constexpr ratio(ratio&&) noexcept = default;
  constexpr ratio& operator=(const ratio&) = default;
  constexpr ratio& operator=(ratio&&) noexcept = default;

  constexpr value_type numerator() const noexcept { return m_numerator; }
  constexpr value_type denominator() const noexcept { return m_denominator; }

  constexpr bool operator==(const ratio& other) const noexcept = default;

  constexpr std::strong_ordering operator<=>(const ratio& other) const {
    // See https://en.cppreference.com/w/cpp/numeric/ratio/ratio_less
    // TODO: We might need to deal with overflows here.
    return (m_numerator * other.m_denominator) <=>
           (other.m_numerator * m_denominator);
  }

  constexpr ratio operator+() const { return *this; }

  constexpr ratio operator-() const {
    return {-m_numerator, m_denominator, no_check{}, no_simplify{}};
  }

  constexpr ratio& operator+=(const ratio& other) {
    // Num == R1::num * R2::den + R2::num * R1::den
    // Den == R1::den * R2::den
    m_numerator =
        m_numerator * other.m_denominator + other.m_numerator * m_denominator;
    m_denominator *= other.m_denominator;
    simplify();
    return *this;
  }
  constexpr ratio& operator-=(const ratio& other) {
    // Num == R1::num * R2::den - R2::num * R1::den
    // Den == R1::den * R2::den
    m_numerator =
        m_numerator * other.m_denominator - other.m_numerator * m_denominator;
    m_denominator *= other.m_denominator;
    simplify();
    return *this;
  }

  constexpr ratio& operator*=(const ratio& other) {
    // Num == R1::num * R2::num
    // Den == R1::den * R2::den
    m_numerator *= other.m_numerator;
    m_denominator *= other.m_denominator;
    simplify();
    return *this;
  }

  constexpr ratio& operator/=(const ratio& other) {
    // Num == R1::num * R2::den
    // Den == R1::den * R2::num
    m_numerator *= other.m_denominator;
    m_denominator *= other.m_numerator;
    simplify();
    return *this;
  }

#define AOC_RATIO_OPERATOR(op, op_eq)                                          \
  friend constexpr ratio operator op(const ratio& lhs, const ratio& rhs) {     \
    return ratio{lhs} op_eq rhs;                                               \
  }
  AOC_RATIO_OPERATOR(+, +=);
  AOC_RATIO_OPERATOR(-, -=);
  AOC_RATIO_OPERATOR(*, *=);
  AOC_RATIO_OPERATOR(/, /=);
#undef AOC_RATIO_OPERATOR

  template <std::floating_point F>
  constexpr explicit operator F() const noexcept {
    return static_cast<F>(m_numerator) / static_cast<F>(m_denominator);
  }

  constexpr explicit operator value_type() const noexcept {
    return m_numerator / m_denominator;
  }

 private:
  constexpr ratio(value_type numerator_, value_type denominator_, no_check,
                  no_simplify)
      : m_numerator(numerator_), m_denominator(denominator_) {
    // Doesn't perform any checks because we're constructing
    // from an already valid instance
  }

  constexpr void simplify() {
    auto gcd = std::gcd(m_numerator, m_denominator);
    m_numerator /= gcd;
    m_denominator /= gcd;
  }

  value_type m_numerator;
  value_type m_denominator;
};

static_assert(ratio{4, 2} == ratio{2, 1});
static_assert(ratio{4, 2} == 2);
static_assert(3 == ratio{6, 2});
static_assert(ratio{6, 2} != 2);
static_assert(2 <= ratio{4, 2});
static_assert(2 < ratio{5, 2});

} // namespace aoc

#endif // AOC_RATIO_H
