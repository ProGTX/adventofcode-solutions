#ifndef AOC_FUNCTIONAL_H
#define AOC_FUNCTIONAL_H

#include "compiler.h"

#ifndef AOC_MODULE_SUPPORT
#ifndef AOC_IMPORT_STD
#include <concepts>
#include <functional>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#endif
#endif

AOC_EXPORT_NAMESPACE(aoc) {

template <class T = int>
constexpr std::function<T(T, T)> get_binary_op(char op) {
  switch (op) {
    case '+':
      return std::plus{};
    case '*':
      return std::multiplies{};
    case '-':
      return std::minus{};
    case '/':
      return std::divides{};
    default:
      throw std::runtime_error("Invalid operation " + std::string{op});
  }
}

template <class T = int>
constexpr std::function<T(T, T)> get_inverse_binary_op(char op) {
  switch (op) {
    case '+':
      return std::minus{};
    case '*':
      return std::divides{};
    case '-':
      return std::plus{};
    case '/':
      return std::multiplies{};
    default:
      throw std::runtime_error("Invalid operation " + std::string{op});
  }
}

template <class T>
struct equal_to_value {
  T value;

  template <std::equality_comparable_with<T> U>
  constexpr bool operator()(U&& other) const {
    return value == other;
  }
};

template <class T>
struct not_equal_to_value {
  T value;

  template <std::equality_comparable_with<T> U>
  constexpr bool operator()(U&& other) const {
    return value != other;
  }
};

template <class return_t>
struct transform_cast {
  template <class T>
  constexpr return_t operator()(T&& val) {
    return static_cast<return_t>(std::forward<T>(val));
  }
};

template <class T>
struct constant_value {
  using value_type = T;
  value_type value;

  template <class... Args>
  constexpr value_type& operator()(Args&&...) {
    return value;
  }
  template <class... Args>
  constexpr const value_type& operator()(Args&&...) const {
    return value;
  }
};

template <class... Ts>
struct overload : Ts... {
  using Ts::operator()...;
};
template <class... Ts>
overload(Ts...) -> overload<Ts...>;

template <class Variant, class... VisitorTs>
constexpr auto match(Variant&& var, VisitorTs&&... visitor) {
  return std::visit(overload{std::forward<VisitorTs>(visitor)...},
                    std::forward<Variant>(var));
}

namespace detail {
template <class T>
constexpr const auto as_consteval_helper =
    [](T&& value) static AOC_CONSTEVAL { return std::forward<T>(value); };

} // namespace detail

/// Forces constant evaluation of the value expression
///
/// Only needed because constinit cannot be used on local variables
template <class T>
constexpr auto as_consteval(T&& value) -> decltype(auto) {
  return detail::as_consteval_helper<T>(std::forward<T>(value));
};

} // AOC_EXPORT_NAMESPACE(aoc)

#endif // AOC_FUNCTIONAL_H
