#pragma once

#include "point.h"

#include <iterator>
#include <type_traits>
#include <utility>

// https://en.cppreference.com/w/cpp/utility/to_underlying
template <class Enum>
constexpr std::underlying_type_t<Enum> to_underlying(Enum e) noexcept {
  return static_cast<std::underlying_type_t<Enum>>(e);
}

// https://www.internalpointers.com/post/writing-custom-iterators-modern-cpp
template <class T>
struct ranged_iterator {
  using iterator_category = std::random_access_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = T;
  using pointer = T*;
  using reference = T&;

  ranged_iterator(pointer ptr, difference_type diff)
      : m_ptr{ptr}, m_diff{diff} {}

  reference operator*() const { return *m_ptr; }

  pointer operator->() { return m_ptr; }

  ranged_iterator& operator++() {
    m_ptr += m_diff;
    return *this;
  }
  ranged_iterator operator++(int) {
    ranged_iterator tmp = *this;
    ++(*this);
    return tmp;
  }

  friend bool operator==(const ranged_iterator& lhs,
                         const ranged_iterator& rhs) {
    return (lhs.m_diff == rhs.m_diff) && (lhs.m_ptr == rhs.m_ptr);
  };
  friend bool operator!=(const ranged_iterator& lhs,
                         const ranged_iterator& rhs) {
    return !(lhs == rhs);
  };

 private:
  pointer m_ptr;
  difference_type m_diff;
};
template <class T>
ranged_iterator(T*, std::ptrdiff_t)->ranged_iterator<T>;

struct min_max_helper {
  point min_value{2'000'000'000, 2'000'000'000};
  point max_value{0, 0};

  constexpr void update(const point& p) {
    if (p.x < min_value.x) {
      min_value.x = p.x;
    }
    if (p.y < min_value.y) {
      min_value.y = p.y;
    }
    if (p.x > max_value.x) {
      max_value.x = p.x;
    }
    if (p.y > max_value.y) {
      max_value.y = p.y;
    }
  };
};
