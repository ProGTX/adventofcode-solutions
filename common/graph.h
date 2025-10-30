#ifndef AOC_GRAPH_H
#define AOC_GRAPH_H

#include "compiler.h"

#ifndef AOC_MODULE_SUPPORT
#include <algorithm>
#include <concepts>
#include <exception>
#include <iostream>
#include <memory>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>
#endif

AOC_EXPORT_NAMESPACE(aoc) {

template <class T, class CRTP, class String = std::string>
class graph {
 public:
  using child_ptr_t = std::unique_ptr<CRTP>;
  using children_t = std::vector<child_ptr_t>;
  using name_t = String;

  using value_type = T;
  using iterator = typename children_t::iterator;
  using const_iterator = typename children_t::const_iterator;

  static_assert(
      !std::same_as<T, name_t>,
      "This class assumes strings are only used for names, not for values");

 protected:
  constexpr graph(CRTP* parent, name_t name, T value, bool is_leaf)
      : m_parent{parent},
        m_name{std::move(name)},
        m_value{std::move(value)},
        m_is_leaf{is_leaf} {}

 public:
  constexpr CRTP* get_parent() const { return m_parent; }

  constexpr CRTP* get_child(
      const std::equality_comparable_with<value_type> auto& value) const {
    auto it = std::ranges::find_if(m_children, [&](auto& child_ptr) {
      return (child_ptr->m_value == value);
    });
    if (it == std::end(m_children)) {
      return nullptr;
    }
    return it->get();
  }

  constexpr CRTP* get_child(
      const std::equality_comparable_with<name_t> auto& name) const {
    auto it = std::ranges::find_if(m_children, [&](auto& child_ptr) {
      return (child_ptr->m_name == name);
    });
    if (it == std::end(m_children)) {
      return nullptr;
    }
    return it->get();
  }

  constexpr CRTP* add_child(child_ptr_t child) {
    if (m_is_leaf) {
      throw std::runtime_error("Cannot add nodes to leaf");
    }
    m_children.push_back(std::move(child));
    return m_children.back().get();
  }

  constexpr iterator begin() { return std::begin(m_children); }
  constexpr const_iterator begin() const { return std::begin(m_children); }

  constexpr iterator end() { return std::end(m_children); }
  constexpr const_iterator end() const { return std::end(m_children); }

  constexpr bool is_leaf() const { return m_is_leaf; }

  std::string_view name() const { return m_name; }

  constexpr T& value() { return m_value; }
  constexpr const T& value() const { return m_value; }

  std::ostream& print(std::ostream& out = std::cout) const {
    if (this->is_leaf()) {
      out << m_value;
      return out;
    }
    out << '[';
    for (const auto& child : m_children) {
      child->print(out) << ',';
    }
    out << ']';
    return out;
  }

 private:
  children_t m_children;
  CRTP* m_parent{nullptr};
  name_t m_name;
  T m_value{};
  bool m_is_leaf{false};
};

} // AOC_EXPORT_NAMESPACE(aoc)

#endif // AOC_GRAPH_H
