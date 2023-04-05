#pragma once

#include "assert.h"

#include <array>
#include <iterator>
#include <type_traits>
#include <utility>

// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0843r2.html
template <class T, size_t N>
class static_vector {
 private:
  // Not quite the proper way to implement it, but good enough for here
  using container_type = std::array<T, N>;

 public:
  using value_type = T;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = value_type&;
  using const_reference = const value_type&;
  using size_type = size_t;
  using difference_type = std::make_signed_t<size_type>;
  using iterator = container_type::iterator; // see [container.requirements]
  using const_iterator =
      container_type::const_iterator; // see [container.requirements]
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  // Iterators
  constexpr iterator begin() noexcept { return m_data.begin(); }
  constexpr const_iterator begin() const noexcept { return m_data.begin(); }
  constexpr iterator end() noexcept { return m_data.begin() + m_size; }
  constexpr const_iterator end() const noexcept {
    return m_data.begin() + m_size;
  }
  constexpr reverse_iterator rbegin() noexcept {
    return m_data.rbegin() + (N - m_size);
  }
  constexpr const_reverse_iterator rbegin() const noexcept {
    return m_data.rbegin() + (N - m_size);
  }
  constexpr reverse_iterator rend() noexcept { return m_data.rend(); }
  constexpr const_reverse_iterator rend() const noexcept {
    return m_data.rend();
  }
  constexpr const_iterator cbegin() noexcept { return m_data.cbegin(); }
  constexpr const_iterator cend() const noexcept { return m_data.cend(); }
  constexpr const_reverse_iterator crbegin() noexcept {
    return m_data.crbegin() + (N - m_size);
  }
  constexpr const_reverse_iterator crend() const noexcept {
    return m_data.crend();
  }

  // Size/capacity:
  constexpr bool empty() const noexcept { return m_size == 0; }
  constexpr size_type size() const noexcept { return m_size; }
  static constexpr size_type max_size() noexcept { return N; }
  static constexpr size_type capacity() noexcept { return N; }
  constexpr void resize(size_type sz) {
    AOC_ASSERT(sz < N, "Exceeded capacity on resize");
    m_size = sz;
  }
  constexpr void resize(size_type sz, const value_type& c) {
    const auto old_size = m_size;
    this->resize(sz);
    for (int i = old_size; i < sz; ++i) {
      m_data[i] = c;
    }
  }

  // Element and data access:
  constexpr reference operator[](size_type n) {
    AOC_ASSERT(n < m_size, "Out of bounds access on subscript op");
    return m_data[n];
  }
  constexpr const_reference operator[](size_type n) const {
    AOC_ASSERT(n < m_size, "Out of bounds access on subscript op");
    return m_data[n];
  }
  constexpr reference front() {
    AOC_ASSERT(m_size > 0, "No elements for front");
    return &m_data[0];
  }
  constexpr const_reference front() const {
    AOC_ASSERT(m_size > 0, "No elements for front");
    return &m_data[0];
  }
  constexpr reference back() {
    AOC_ASSERT(m_size > 0, "No elements for back");
    return &m_data[m_size - 1];
  }
  constexpr const_reference back() const {
    AOC_ASSERT(m_size > 0, "No elements for back");
    return &m_data[m_size - 1];
  }
  constexpr T* data() noexcept { return &m_data; }
  constexpr const T* data() const noexcept { return &m_data; }

  // Add elements
  template <class... Args>
  constexpr reference emplace_back(Args&&... args) {
    this->push_back(value_type{std::forward<Args>(args)...});
    return this->back();
  }
  constexpr void push_back(const value_type& x) {
    AOC_ASSERT(m_size < N, "Exceeded capacity on push_back");
    m_data[m_size] = x;
    ++m_size;
  }
  constexpr void push_back(value_type&& x) {
    AOC_ASSERT(m_size < N, "Exceeded capacity on push_back");
    m_data[m_size] = std::move(x);
    ++m_size;
  }

  // Remove elements
  constexpr void pop_back() {
    AOC_ASSERT(m_size > 0, "No element to pop");
    --m_size;
  }
  constexpr void clear() noexcept { m_size = 0; }

 private:
  container_type m_data;
  size_t m_size = 0;
};
