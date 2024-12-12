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
    return this->insert_generic(cend(), value);
  }
  constexpr std::pair<iterator, bool> insert(value_type&& value) { // 2
    return this->insert_generic(cend(), std::move(value));
  }
  iterator insert(const_iterator pos, const value_type& value) { // 3
    auto [it, inserted] = this->insert_generic(pos, value);
    return it;
  }
  iterator insert(const_iterator pos, value_type&& value) { // 4
    auto [it, inserted] = this->insert_generic(pos, std::move(value));
    return it;
  }

  template <class... Args>
  constexpr std::pair<iterator, bool> emplace(Args&&... args) {
    return this->insert_generic(cend(),
                                value_type(std::forward<Args>(args)...));
  }

  // erase()
  constexpr iterator erase(iterator pos) // 1
    requires(!std::same_as<iterator, const_iterator>)
  {
    return this->erase(static_cast<const_iterator>(pos));
  }
  constexpr iterator erase(const_iterator pos) { // 2
    return key_container.erase(pos);
  }
  constexpr size_type erase(const Key& key) { // 4
    auto it = this->find(key);
    if (it != end()) {
      this->erase(it);
      return 1;
    } else {
      return 0;
    }
  }

  /////////
  // Lookup

  // find()
  constexpr iterator find(const Key& key) { // 1
    auto it = this->lower_bound(key);
    if ((it == end()) || (*it == key)) {
      return it;
    } else {
      return end();
    }
  }
  constexpr const_iterator find(const Key& key) const { // 2
    auto it = this->lower_bound(key);
    if ((it == end()) || (*it == key)) {
      return it;
    } else {
      return end();
    }
  }

  constexpr bool contains(const Key& key) const {
    return this->find(key) != end();
  }

 private:
  constexpr iterator lower_bound(const Key& key) {
    return std::ranges::lower_bound(key_container, key, compare);
  }
  constexpr const_iterator lower_bound(const Key& key) const {
    return std::ranges::lower_bound(key_container, key, compare);
  }

  template <class V = value_type>
  constexpr std::pair<iterator, bool> insert_generic(
      [[maybe_unused]] const_iterator pos, V&& key) {
    // NOTE: pos is just a hint, we ignore it for now
    auto existing_it = this->lower_bound(key);
    if ((existing_it == cend()) || (*existing_it != key)) {
      // NOTE: This could potentially be made faster if using another container
      auto it = key_container.insert(existing_it, std::forward<V>(key));
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
