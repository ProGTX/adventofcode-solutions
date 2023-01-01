#pragma once

#include <algorithm>
#include <concepts>
#include <functional>
#include <ranges>
#include <utility>
#include <vector>

template <class T, class Compare = std::less<T>>
requires std::equality_comparable<T> class sorted_flat_set {
 private:
  using data_t = std::vector<T>;

 public:
  using value_type = data_t::value_type;
  using iterator = data_t::iterator;
  using const_iterator = data_t::const_iterator;

  constexpr sorted_flat_set() : sorted_flat_set{{}, Compare{}} {}

  constexpr explicit sorted_flat_set(const Compare& comp)
      : sorted_flat_set{{}, comp} {}

  constexpr sorted_flat_set(data_t init, const Compare& comp = Compare{})
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

  constexpr size_t erase(
      const std::equality_comparable_with<value_type> auto& value) {
    auto it = this->find(value);
    if (it == this->end()) {
      return 0;
    }
    m_data.erase(it);
    this->sort();
    return 1;
  }

  constexpr void sort() { std::ranges::sort(m_data, m_comparator); }

 private:
  data_t m_data;
  Compare m_comparator;
};
