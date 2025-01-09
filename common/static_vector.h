#ifndef AOC_STATIC_VECTOR_H
#define AOC_STATIC_VECTOR_H

#include "assert.h"
#include "ranges.h"

#include <algorithm>
#include <array>
#include <iterator>
#include <type_traits>
#include <utility>

namespace aoc {

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
  using iterator = typename container_type::iterator; //[container.requirements]
  using const_iterator =
      typename container_type::const_iterator; // see [container.requirements]
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  // 5.2, copy/move construction:
  constexpr static_vector() noexcept = default;

  constexpr static_vector(const static_vector& other) noexcept(
      std::is_nothrow_copy_constructible_v<value_type>) = default;
  constexpr static_vector(static_vector&& other) noexcept(
      std::is_nothrow_move_constructible_v<value_type>) = default;
  constexpr static_vector& operator=(const static_vector& other) noexcept(
      std::is_nothrow_copy_assignable_v<value_type>) = default;
  constexpr static_vector& operator=(static_vector&& other) noexcept(
      std::is_nothrow_move_assignable_v<value_type>) = default;

  constexpr explicit static_vector(size_type n) : m_size{n} {
    this->assert_size(n);
  }
  constexpr static_vector(size_type n, const value_type& value) : m_size{n} {
    this->assert_size(n);
    std::ranges::fill_n(std::begin(m_data), n, value);
  }
  template <std::input_iterator I, std::sentinel_for<I> S>
  constexpr static_vector(I first, S last)
      : m_size{static_cast<size_t>(std::ranges::distance(first, last))} {
    std::ranges::copy(first, last, std::begin(m_data));
  }
  constexpr static_vector(std::initializer_list<value_type> il)
      : static_vector(std::ranges::begin(il), std::ranges::end(il)) {}

  template <std::ranges::range R>
  // requires (container-compatible-range<R, T>)
  constexpr static_vector(ranges::from_range_t, R&& rg)
      : static_vector(std::ranges::begin(rg), std::ranges::end(rg)) {}

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
  constexpr const_iterator cbegin() noexcept { return this->begin(); }
  constexpr const_iterator cend() const noexcept { return this->end(); }
  constexpr const_reverse_iterator crbegin() noexcept { return this->rbegin(); }
  constexpr const_reverse_iterator crend() const noexcept {
    return this->rend();
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
    return m_data[0];
  }
  constexpr const_reference front() const {
    AOC_ASSERT(m_size > 0, "No elements for front");
    return m_data[0];
  }
  constexpr reference back() {
    AOC_ASSERT(m_size > 0, "No elements for back");
    return m_data[m_size - 1];
  }
  constexpr const_reference back() const {
    AOC_ASSERT(m_size > 0, "No elements for back");
    return m_data[m_size - 1];
  }
  constexpr T* data() noexcept { return m_data.data(); }
  constexpr const T* data() const noexcept { return m_data.data(); }

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

  constexpr iterator insert(const_iterator pos, const T& value) {
    auto non_const_pos = begin() + std::distance(cbegin(), pos);
    this->push_back(value);
    // rotate_right
    std::ranges::rotate(non_const_pos + 1, end(), end() - 1);
    return non_const_pos;
  }
  constexpr iterator insert(const_iterator pos, T&& value) {
    // TODO
    return insert(pos, value);
  }

 protected:
  constexpr void assert_size(size_type n) const {
    AOC_ASSERT(
        n <= N,
        "Cannot construct a static_vector larger than the allocated storage");
  }

 private:
  container_type m_data;
  size_t m_size = 0;
};

constexpr auto impl_test_insert(int pos) {
  auto vec = static_vector<int, 4>{2, 4, 6};
  vec.insert(vec.begin() + pos, 42);
  return vec;
}
#ifndef AOC_COMPILER_MSVC
static_assert(std::ranges::equal(std::array{42, 2, 4, 6}, impl_test_insert(0)));
static_assert(std::ranges::equal(std::array{2, 42, 4, 6}, impl_test_insert(1)));
static_assert(std::ranges::equal(std::array{2, 4, 42, 6}, impl_test_insert(2)));
static_assert(std::ranges::equal(std::array{2, 4, 6, 42}, impl_test_insert(3)));
#endif

} // namespace aoc

#endif // AOC_STATIC_VECTOR_H
