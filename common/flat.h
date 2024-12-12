#ifndef AOC_FLAT_H
#define AOC_FLAT_H

#include "concepts.h"

#include <algorithm>
#include <concepts>
#include <functional>
#include <iterator>
#include <ranges>
#include <utility>
#include <vector>

namespace aoc {

template <class T, class Compare = std::less<T>>
  requires std::equality_comparable<T>
class sorted_flat_set {

 public:
  using container_type = std::vector<T>;
  using value_type = typename container_type::value_type;
  using iterator = typename container_type::iterator;
  using const_iterator = typename container_type::const_iterator;

  constexpr sorted_flat_set() : sorted_flat_set{{}, Compare{}} {}

  constexpr explicit sorted_flat_set(const Compare& comp)
      : sorted_flat_set{{}, comp} {}

  constexpr sorted_flat_set(container_type init,
                            const Compare& comp = Compare{})
      : m_data{init}, m_comparator{comp} {
    this->sort();
  }

  constexpr iterator begin() noexcept { return m_data.begin(); }
  constexpr const_iterator begin() const noexcept { return m_data.begin(); }

  constexpr iterator end() noexcept { return m_data.end(); }
  constexpr const_iterator end() const noexcept { return m_data.end(); }

  constexpr iterator find(
      const std::equality_comparable_with<value_type> auto& value) {
    return std::ranges::find_if(
        m_data, [&](const value_type& v) { return v == value; });
  }
  constexpr const_iterator find(
      const std::equality_comparable_with<value_type> auto& value) const {
    return std::ranges::find_if(
        m_data, [&](const value_type& v) { return v == value; });
  }

  constexpr bool contains(
      const std::equality_comparable_with<value_type> auto& value) const {
    return this->find(value) != this->end();
  }

  constexpr size_t size() const noexcept { return m_data.size(); }

  [[nodiscard]] constexpr bool empty() const noexcept { return m_data.empty(); }

  constexpr std::pair<iterator, bool> insert(
      std::equality_comparable_with<value_type> auto&& value) {
    auto it = this->find(value);
    if (it != this->end()) {
      return {it, false};
    }
    m_data.push_back(value_type{value});
    this->sort();
    return {this->find(value), true};
  }

  template <std::equality_comparable_with<value_type> value_t = value_type>
  constexpr iterator insert(const_iterator, value_t&& value) {
    auto [it, success] = this->insert(std::forward<value_t>(value));
    return it;
  }

  template <class... Args>
  constexpr std::pair<iterator, bool> emplace(Args&&... args) {
    auto value = T{std::forward<Args>(args)...};
    return this->insert(std::move(value));
  }

  constexpr std::pair<iterator, bool> update(
      std::equality_comparable_with<value_type> auto&& value) {
    auto it = this->find(value);
    if (it == this->end()) {
      m_data.push_back(value_type{value});
    } else {
      *it = value;
    }
    this->sort();
    return {this->find(value), true};
  }

  constexpr iterator erase(const_iterator pos) { return m_data.erase(pos); }
  constexpr iterator erase(iterator pos)
    requires(!std::same_as<iterator, const_iterator>)
  {
    return m_data.erase(pos);
  }

  constexpr size_t erase(
      const std::equality_comparable_with<value_type> auto& value) {
    auto it = this->find(value);
    if (it == this->end()) {
      return 0;
    }
    this->erase(it);
    return 1;
  }

  constexpr void sort() { std::ranges::sort(m_data, m_comparator); }

 private:
  container_type m_data;
  Compare m_comparator;
};

// Barebones implementation of
// https://en.cppreference.com/w/cpp/container/flat_set, but with constexpr
template <class Key, class Compare = std::less<Key>,
          class KeyContainer = std::vector<Key>>
class flat_set {
 private:
  // NOTE: This container was designed with support for std::vector only,
  // though I'd expect static_vector to work as well

 public:
  using container_type = KeyContainer;
  using key_type = Key;
  using value_type = Key;
  using key_compare = Compare;
  using value_compare = Compare;
  using reference = value_type&;
  using const_reference = const value_type&;
  using size_type = typename container_type::size_type;
  using difference_type = typename container_type::difference_type;
  using iterator = typename container_type::iterator;
  using const_iterator = typename container_type::const_iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

 private:
  template <class flat_set_t>
  using select_it_t =
      std::conditional_t<std::is_const_v<flat_set_t>, const_iterator, iterator>;

 public:
  ///////////////
  // Constructors

  constexpr flat_set() : flat_set(key_compare()) {} // 1
  constexpr explicit flat_set(const key_compare& comp)
      : key_container(), compare(comp) {} // 10

  ////////////
  // Iterators

  constexpr iterator begin() noexcept { return key_container.begin(); }
  constexpr const_iterator begin() const noexcept {
    return key_container.begin();
  }
  constexpr const_iterator cbegin() const noexcept {
    return key_container.begin();
  }

  constexpr iterator end() noexcept { return key_container.end(); }
  constexpr const_iterator end() const noexcept { return key_container.end(); }
  constexpr const_iterator cend() const noexcept { return key_container.end(); }

  ///////////
  // Capacity

  [[nodiscard]] constexpr bool empty() const noexcept {
    return key_container.empty();
  }
  constexpr size_type size() const noexcept { return key_container.size(); }

  ////////////
  // Modifiers

  // insert()
  constexpr std::pair<iterator, bool> insert(const value_type& value) { // 1
    return this->insert_generic(value);
  }
  constexpr std::pair<iterator, bool> insert(value_type&& value) { // 2
    return this->insert_generic(std::move(value));
  }

  /////////
  // Lookup

  // find()
  constexpr iterator find(const Key& key) { // 1
    return find_generic(*this, key);
  }
  constexpr const_iterator find(const Key& key) const { // 2
    return find_generic(*this, key);
  }

 private:
  template <class flat_set_t>
  constexpr friend select_it_t<flat_set_t> find_generic(flat_set_t&& set,
                                                        const Key& key) {
    auto it = std::ranges::lower_bound(set.key_container, key, set.compare);
    if ((it == set.end()) || (*it == key)) {
      return it;
    } else {
      return set.end();
    }
  }

  template <class V = value_type>
  constexpr std::pair<iterator, bool> insert_generic(V&& value) {
    auto existing_it = this->find(value);
    if (existing_it == this->end()) {
      // NOTE: This could potentially be made faster if using another container
      auto it = key_container.insert(existing_it, std::forward<V>(value));
      return {it, true};
    } else {
      return {existing_it, false};
    }
  }

 private:
  container_type key_container; // AKA c
  key_compare compare;
};

} // namespace aoc

#endif // AOC_FLAT_H
