#pragma once

#include "point.h"

#include <exception>
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

  constexpr ranged_iterator(pointer ptr, difference_type diff)
      : m_ptr{ptr}, m_diff{diff} {}

  constexpr reference operator*() const { return *m_ptr; }

  constexpr pointer operator->() { return m_ptr; }

  constexpr ranged_iterator& operator++() {
    m_ptr += m_diff;
    return *this;
  }
  constexpr ranged_iterator operator++(int) {
    ranged_iterator tmp = *this;
    ++(*this);
    return tmp;
  }

  constexpr friend bool operator==(const ranged_iterator& lhs,
                                   const ranged_iterator& rhs) {
    return (lhs.m_diff == rhs.m_diff) && (lhs.m_ptr == rhs.m_ptr);
  };

 private:
  pointer m_ptr;
  difference_type m_diff;
};
template <class T>
ranged_iterator(T*, std::ptrdiff_t)->ranged_iterator<T>;

template <class Container, bool extra_edge = false>
struct cyclic_iterator {
  using iterator_category = std::random_access_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = Container::value_type;
  using pointer = Container::pointer;
  using reference = std::conditional_t<std::is_const_v<Container>,
                                       typename Container::const_reference,
                                       typename Container::reference>;

  using container_type = Container;
  using iterator = std::conditional_t<std::is_const_v<Container>,
                                      typename Container::const_iterator,
                                      typename Container::iterator>;
  using const_iterator = Container::const_iterator;

  constexpr cyclic_iterator(container_type&&, iterator it) = delete;
  constexpr cyclic_iterator(container_type& container, iterator it)
      : m_begin{std::begin(container)},
        m_end{std::end(container)},
        m_it{it},
        m_size{std::distance(m_begin, m_end)} {
    if (m_size == 0) {
      throw std::runtime_error("No point having an empty cyclic operator");
    }
  }

  constexpr cyclic_iterator(container_type&& container) = delete;
  constexpr cyclic_iterator(container_type& container)
      : cyclic_iterator{container, std::begin(container)} {}

  constexpr reference operator*() const { return *m_it; }
  constexpr pointer operator->() { return m_it.operator->(); }

  constexpr cyclic_iterator& operator+=(difference_type diff) {
    if (diff == 0) {
      return *this;
    }
    if constexpr (!extra_edge) {
      auto pos = std::distance(m_begin, m_it);
      pos = (m_size + (pos + (diff % m_size))) % m_size;
      m_it = m_begin + pos;
    } else {
      if (diff < 0) {
        for (int i = 0; i > diff; --i) {
          this->operator--();
        }
      } else {
        for (int i = 0; i < diff; ++i) {
          this->operator++();
        }
      }
    }

    return *this;
  }
  constexpr friend cyclic_iterator operator+(cyclic_iterator lhs,
                                             difference_type diff) {
    lhs += diff;
    return lhs;
  }

  constexpr cyclic_iterator& operator-=(difference_type diff) {
    return this->operator+=(-diff);
  }
  constexpr friend cyclic_iterator operator-(cyclic_iterator lhs,
                                             difference_type diff) {
    lhs -= diff;
    return lhs;
  }

  constexpr cyclic_iterator& operator++() {
    ++m_it;
    if (m_it == m_end) {
      m_it = m_begin;
      if constexpr (extra_edge) {
        ++m_it;
      }
    }
    return *this;
  }
  constexpr cyclic_iterator operator++(int) {
    cyclic_iterator tmp = *this;
    ++(*this);
    return tmp;
  }

  constexpr cyclic_iterator& operator--() {
    if (m_it == m_begin) {
      m_it = m_end;
      if constexpr (extra_edge) {
        --m_it;
      }
    }
    --m_it;
    if constexpr (extra_edge) {
      if (m_it == m_begin) {
        m_it = m_end - 1;
      }
    }
    return *this;
  }
  constexpr cyclic_iterator operator--(int) {
    cyclic_iterator tmp = *this;
    ++(*this);
    return tmp;
  }

  constexpr friend bool operator==(const cyclic_iterator& lhs,
                                   const cyclic_iterator& rhs) {
    return (lhs.m_it == rhs.m_it) && (lhs.m_begin == rhs.m_begin) &&
           (lhs.m_end == rhs.m_end) && (lhs.m_size == rhs.m_size);
  };

  constexpr operator iterator() const { return m_it; }

 private:
  iterator m_begin;
  iterator m_end;
  iterator m_it;
  difference_type m_size;
};
template <class Container>
cyclic_iterator(Container&)->cyclic_iterator<Container>;
template <class Container>
cyclic_iterator(const Container&)->cyclic_iterator<const Container>;
template <class Container>
cyclic_iterator(Container&, typename Container::iterator)
    ->cyclic_iterator<Container>;
template <class Container>
cyclic_iterator(const Container&, typename Container::iterator)
    ->cyclic_iterator<const Container>;
template <class Container>
cyclic_iterator(Container&, typename Container::const_iterator)
    ->cyclic_iterator<Container>;
template <class Container>
cyclic_iterator(const Container&, typename Container::const_iterator)
    ->cyclic_iterator<const Container>;

template <class map_iterator>
struct map_value_iterator {
 protected:
  using map_iterator_t = map_iterator;
  using map_value_t = map_iterator_t::value_type;

 public:
  using iterator_category = std::random_access_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = decltype(std::declval<map_value_t>().second);
  using pointer = value_type*;
  using reference = value_type&;

  constexpr map_value_iterator(map_iterator map_it) : m_map_it{map_it} {}

  constexpr reference operator*() const { return m_map_it->second; }

  constexpr pointer operator->() { return &m_map_it->second; }

  constexpr map_value_iterator& operator++() {
    ++m_map_it;
    return *this;
  }
  constexpr map_value_iterator operator++(int) {
    map_value_iterator tmp = *this;
    ++(*this);
    return tmp;
  }

  constexpr friend bool operator==(const map_value_iterator& lhs,
                                   const map_value_iterator& rhs) {
    return lhs.m_map_it == rhs.m_map_it;
  };

 private:
  map_iterator_t m_map_it;
};
template <class map_iterator>
map_value_iterator(map_iterator)->map_value_iterator<map_iterator>;

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
