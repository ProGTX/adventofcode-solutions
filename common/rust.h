#pragma once

// This file contains #defines and type aliases
// that make C++ look more like Rust,
// aka Carcinization of C++

#include <cstdint>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#define let const auto
#define fn [[nodiscard]] constexpr auto

using i32 = std::int32_t;
using u32 = std::uint32_t;
using usize = std::size_t;

using String = std::string;
using str = std::string_view;

template <class T>
using Option = std::optional<T>;
template <class T>
using Vec = std::vector<T>;

template <std::weakly_incrementable W,
          std::semiregular Bound = std::unreachable_sentinel_t>
using Range = std::ranges::iota_view<W, Bound>;
