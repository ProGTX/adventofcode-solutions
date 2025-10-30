#ifndef AOC_FLAT_H
#define AOC_FLAT_H

#include "compiler.h"
#include "concepts.h"

#ifndef AOC_MODULE_SUPPORT
#include <algorithm>
#include <compare>
#include <concepts>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <utility>
#include <vector>
#endif

AOC_EXPORT_NAMESPACE(aoc) {

// Barebones implementation of
// https://en.cppreference.com/w/cpp/container/flat_set, but with constexpr
// NOTE: Can be replaced by std::flat_set in C++26
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

  constexpr flat_set() : flat_set(key_compare()) {}    // 1
  constexpr explicit flat_set(const key_compare& comp) // 10
      : key_container(), compare(comp) {}
  template <class InputIter>
  constexpr flat_set(InputIter first, InputIter last,
                     const key_compare& comp = key_compare()) // 13
      : compare(comp) {
    this->insert(first, last);
  }
  constexpr flat_set(std::initializer_list<value_type> init,
                     const key_compare& comp = key_compare()) // 23
      : flat_set(init.begin(), init.end(), comp) {}

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
  constexpr iterator insert(const_iterator pos, const value_type& value) { // 3
    auto [it, inserted] = this->insert_generic(pos, value);
    return it;
  }
  constexpr iterator insert(const_iterator pos, value_type&& value) { // 4
    auto [it, inserted] = this->insert_generic(pos, std::move(value));
    return it;
  }
  template <class InputIt>
  constexpr void insert(InputIt first, InputIt last) { // 6
    for (; first != last; ++first) {
      this->insert(*first);
    }
  }

  template <class... Args>
  constexpr std::pair<iterator, bool> emplace(Args&&... args) {
    return this->insert_generic(cend(),
                                value_type(std::forward<Args>(args)...));
  }

  constexpr container_type extract() && { return std::move(key_container); }

  /// Preconditions:
  /// 1. The elements of cont must be sorted with respect to compare
  /// 2. cont must not contain equal elements
  constexpr void replace(container_type&& cont) {
    key_container = std::move(cont);
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

  constexpr void clear() noexcept { key_container.clear(); }

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

  constexpr iterator lower_bound(const Key& key) {
    return std::ranges::lower_bound(key_container, key, compare);
  }
  constexpr const_iterator lower_bound(const Key& key) const {
    return std::ranges::lower_bound(key_container, key, compare);
  }

 private:
  template <class V = value_type>
  constexpr std::pair<iterator, bool> insert_generic(
      [[maybe_unused]] const_iterator hint, V&& key) {
    // NOTE: We ignore the hint for now
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

// Barebones implementation of
// https://en.cppreference.com/w/cpp/container/flat_map, but with constexpr
// NOTE: Can be replaced by std::flat_map in C++26
template <class Key, class T, class Compare = std::less<Key>,
          class KeyContainer = std::vector<Key>,
          class MappedContainer = std::vector<T>>
class flat_map {
 public:
  using key_container_type = KeyContainer;
  using mapped_container_type = MappedContainer;
  using key_type = Key;
  using mapped_type = T;
  using value_type = std::pair<key_type, mapped_type>;
  using key_compare = Compare;
  using reference = std::pair<const key_type&, mapped_type&>;
  using const_reference = std::pair<const key_type&, const mapped_type&>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

 private:
  struct detail_iterator_tag {};

  using key_iterator = KeyContainer::iterator;
  using const_key_iterator = KeyContainer::const_iterator;
  using value_iterator = MappedContainer::iterator;
  using const_value_iterator = MappedContainer::const_iterator;

  // arrow_proxy, iter, make_iterator are based on the flatmap_detail class from
  // https://github.com/WG21-SG14/SG14/blob/master/SG14/flat_map.h
  /*
   * Boost Software License - Version 1.0 - August 17th, 2003
   *
   * Permission is hereby granted, free of charge, to any person or organization
   * obtaining a copy of the software and accompanying documentation covered by
   * this license (the "Software") to use, reproduce, display, distribute,
   * execute, and transmit the Software, and to prepare derivative works of the
   * Software, and to permit third-parties to whom the Software is furnished to
   * do so, all subject to the following:
   *
   * The copyright notices in the Software and this entire statement, including
   * the above license grant, this restriction and the following disclaimer,
   * must be included in all copies of the Software, in whole or in part, and
   * all derivative works of the Software, unless such copies or derivative
   * works are solely in the form of machine-executable object code generated by
   * a source language processor.
   *
   * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
   * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
   * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
   * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   * DEALINGS IN THE SOFTWARE.
   */
  template <class Reference>
  struct arrow_proxy {
    constexpr Reference* operator->() { return std::addressof(data_); }

    template <class, class>
    friend class iterator_detail;

    Reference data_;
  };
  template <class KeyIt, class MappedIt>
  class iterator_detail {
   public:
    using difference_type = ptrdiff_t;
    using value_type =
        std::pair<const typename std::iterator_traits<KeyIt>::value_type,
                  typename std::iterator_traits<MappedIt>::value_type>;
    using reference =
        std::pair<typename std::iterator_traits<KeyIt>::reference,
                  typename std::iterator_traits<MappedIt>::reference>;
    using pointer = arrow_proxy<reference>;
    using iterator_category = std::random_access_iterator_tag;

    constexpr iterator_detail() = default;
    constexpr iterator_detail(iterator_detail&&) = default;
    constexpr iterator_detail(const iterator_detail&) = default;
    constexpr iterator_detail& operator=(iterator_detail&&) = default;
    constexpr iterator_detail& operator=(const iterator_detail&) = default;
    constexpr ~iterator_detail() = default;

    // This is the iterator-to-const_iterator implicit conversion.
    template <std::convertible_to<KeyIt> CK, std::convertible_to<MappedIt> CM>
    constexpr iterator_detail(const iterator_detail<CK, CM>& other)
        : kit_(other.private_impl_getkey()),
          vit_(other.private_impl_getmapped()) {}

    constexpr reference operator*() const { return reference{*kit_, *vit_}; }

    constexpr pointer operator->() const {
      return arrow_proxy<reference>{reference{*kit_, *vit_}};
    }

    constexpr iterator_detail& operator++() {
      ++kit_;
      ++vit_;
      return *this;
    }
    constexpr iterator_detail& operator--() {
      --kit_;
      --vit_;
      return *this;
    }
    constexpr iterator_detail operator++(int) {
      iterator_detail result(*this);
      ++*this;
      return result;
    }
    constexpr iterator_detail operator--(int) {
      iterator_detail result(*this);
      --*this;
      return result;
    }
    constexpr iterator_detail& operator+=(ptrdiff_t n) {
      kit_ += n;
      vit_ += n;
      return *this;
    }
    constexpr iterator_detail& operator-=(ptrdiff_t n) {
      kit_ -= n;
      vit_ -= n;
      return *this;
    }
    constexpr reference operator[](ptrdiff_t n) const { return *(*this + n); }
    constexpr friend iterator_detail operator+(iterator_detail it,
                                               ptrdiff_t n) {
      it += n;
      return it;
    }
    constexpr friend iterator_detail operator+(ptrdiff_t n,
                                               iterator_detail it) {
      it += n;
      return it;
    }
    constexpr friend iterator_detail operator-(iterator_detail it,
                                               ptrdiff_t n) {
      it -= n;
      return it;
    }
    constexpr friend ptrdiff_t operator-(const iterator_detail& it,
                                         const iterator_detail& jt) {
      return ptrdiff_t(it.kit_ - jt.kit_);
    }

    constexpr bool operator==(const iterator_detail&) const = default;
    constexpr auto operator<=>(const iterator_detail&) const = default;

    constexpr KeyIt private_impl_getkey() const { return kit_; }
    constexpr MappedIt private_impl_getmapped() const { return vit_; }

    // This constructor is public because the friend below doesn't work
    constexpr explicit iterator_detail(detail_iterator_tag, KeyIt&& kit,
                                       MappedIt&& vit)
        : kit_(static_cast<KeyIt&&>(kit)), vit_(static_cast<MappedIt&&>(vit)) {}

   private:
    KeyIt kit_;
    MappedIt vit_;
  };
  template <class K, class V>
  constexpr iterator_detail<K, V> make_iterator(K kit, V vit) {
    return iterator_detail<K, V>(detail_iterator_tag{}, static_cast<K&&>(kit),
                                 static_cast<V&&>(vit));
  }

  template <class K>
  constexpr auto make_iterator(K kit) {
    auto vit = this->matching_value_iterator(kit);
    using V = decltype(vit);
    return iterator_detail<K, V>(detail_iterator_tag{}, static_cast<K&&>(kit),
                                 static_cast<V&&>(vit));
  }
  template <class K>
  constexpr auto make_iterator(K kit) const {
    auto vit = this->matching_value_iterator(kit);
    using V = decltype(vit);
    return iterator_detail<K, V>(detail_iterator_tag{}, static_cast<K&&>(kit),
                                 static_cast<V&&>(vit));
  }

 public:
  using iterator = iterator_detail<const_key_iterator, value_iterator>;
  using const_iterator =
      iterator_detail<const_key_iterator, const_value_iterator>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  struct containers {
    key_container_type keys;
    mapped_container_type values;
  };

  class value_compare {
   private:
    constexpr value_compare(key_compare c) : comp(c) {}

   public:
    constexpr bool operator()(const const_reference& lhs,
                              const const_reference& rhs) const {
      return comp(lhs.first, rhs.first);
    }

   private:
    key_compare comp;
  };

 public:
  ///////////////
  // Constructors

  constexpr flat_map() // 1
      : flat_map(key_compare()) {}

  constexpr explicit flat_map(const key_compare& comp) // 10
      : compare(comp) {}

  template <class InputIter>
  constexpr flat_map(InputIter first, InputIter last,
                     const key_compare& comp = key_compare()) // 13
      : compare(comp) {
    this->insert(first, last);
  }

  constexpr flat_map(std::initializer_list<value_type> init,
                     const key_compare& comp = key_compare()) // 23
      : flat_map(init.begin(), init.end(), comp) {}

  /////////////////
  // Element access

  constexpr T& at(const Key& key) {
    auto it = this->find(key);
    if (it == end()) {
      throw std::out_of_range("flat_map::at");
    }
    return it->second;
  }
  constexpr const T& at(const Key& key) const {
    auto it = this->find(key);
    if (it == end()) {
      throw std::out_of_range("flat_map::at");
    }
    return it->second;
  }

  constexpr T& operator[](const Key& key) {
    return this->try_emplace(key).first->second;
  }
  constexpr T& operator[](Key&& key) {
    return this->try_emplace(std::move(key)).first->second;
  }

  ////////////
  // Iterators

  constexpr iterator begin() noexcept {
    return this->make_iterator(storage.keys.begin());
  }
  constexpr const_iterator begin() const noexcept {
    return this->make_iterator(storage.keys.begin());
  }
  constexpr const_iterator cbegin() const noexcept {
    return this->make_iterator(storage.keys.begin());
  }

  constexpr iterator end() noexcept {
    return this->make_iterator(storage.keys.end());
  }
  constexpr const_iterator end() const noexcept {
    return this->make_iterator(storage.keys.end());
  }
  constexpr const_iterator cend() const noexcept {
    return this->make_iterator(storage.keys.end());
  }

  ///////////
  // Capacity

  [[nodiscard]] constexpr bool empty() const noexcept {
    return storage.keys.empty();
  }
  constexpr size_type size() const noexcept { return storage.keys.size(); }

  ////////////
  // Modifiers

  template <class... Args>
  constexpr std::pair<iterator, bool> emplace(Args&&... args) {
    return this->insert_generic(storage.keys.cend(),
                                value_type(std::forward<Args>(args)...));
  }

  template <class... Args>
  constexpr iterator emplace_hint([[maybe_unused]] const_iterator hint,
                                  Args&&... args) {
    return this
        ->insert_generic(storage.keys.cend(),
                         value_type(std::forward<Args>(args)...))
        .first;
  }

  constexpr containers extract() && { return std::move(storage); }

  /// Preconditions:
  /// 1. The expression key_cont.size() == mapped_cont.size() is true
  /// 2. The elements of key_cont are sorted with respect to compare
  /// 3. key_cont does not contain equal elements
  constexpr void replace(key_container_type&& key_cont,
                         mapped_container_type&& mapped_cont) {
    AOC_ASSERT(key_cont.size() == mapped_cont.size(),
               "1. Key and value containers must be same size");
    storage.keys = std::move(key_cont);
    storage.values = std::move(mapped_cont);
  }

  constexpr void clear() noexcept {
    storage.keys.clear();
    storage.values.clear();
  }

  // try_emplace
  template <class... Args>
  constexpr std::pair<iterator, bool> try_emplace(const key_type& key,
                                                  Args&&... args) { // 1
    return this->insert_generic(storage.keys.cend(), key,
                                std::forward<Args>(args)...);
  }
  template <class... Args>
  constexpr std::pair<iterator, bool> try_emplace(key_type&& key,
                                                  Args&&... args) { // 2
    return this->insert_generic(storage.keys.cend(), std::move(key),
                                std::forward<Args>(args)...);
  }
  template <class... Args>
  constexpr iterator try_emplace(const_iterator hint, const key_type& key,
                                 Args&&... args) { // 4
    return this->insert_generic(hint, key, std::forward<Args>(args)...);
  }
  template <class... Args>
  constexpr iterator try_emplace(const_iterator hint, key_type&& key,
                                 Args&&... args) { // 5
    return this->insert_generic(hint, std::move(key),
                                std::forward<Args>(args)...);
  }

  // insert
  constexpr std::pair<iterator, bool> insert(const value_type& value) { // 1
    return this->emplace(value);
  }
  constexpr std::pair<iterator, bool> insert(value_type&& value) { // 2
    return this->emplace(std::move(value));
  }
  constexpr iterator insert(const_iterator pos, const value_type& value) { // 3
    return this->emplace_hint(pos, value);
  }
  constexpr iterator insert(const_iterator pos, value_type&& value) { // 4
    return this->emplace_hint(pos, std::move(value));
  }
  template <class InputIt>
  constexpr void insert(InputIt first, InputIt last) { // 7
    for (auto&& elem : std::ranges::subrange(first, last)) {
      this->emplace(std::move(elem));
    }
  }
  constexpr void insert(std::initializer_list<key_type> ilist) { // 9
    this->insert(ilist.begin(), ilist.end());
  }

  // erase
  constexpr iterator erase(iterator position) { // 1
    return this->erase(static_cast<const_iterator>(position));
  }
  constexpr iterator erase(const_iterator pos) { // 2
    auto key_it = storage.keys.erase(pos.private_impl_getkey());
    auto value_it = storage.values.erase(pos.private_impl_getmapped());
    return this->make_iterator(key_it, value_it);
  }
  constexpr size_type erase(const Key& key) { // 4
    auto pos = this->find(key);
    if (pos != end()) {
      this->erase(pos);
      return 1;
    } else {
      return 0;
    }
  }

  /////////
  // Lookup

  constexpr iterator find(const Key& key) {
    auto key_it = this->lower_bound_key(key);
    if ((key_it == storage.keys.end()) || (*key_it == key)) {
      return this->make_iterator(key_it);
    } else {
      return end();
    }
  }
  constexpr const_iterator find(const Key& key) const {
    auto key_it = this->lower_bound_key(key);
    if ((key_it == storage.keys.end()) || (*key_it == key)) {
      return this->make_iterator(key_it);
    } else {
      return end();
    }
  }

  constexpr size_type count(const Key& key) const {
    return static_cast<size_type>(this->contains(key));
  }

  constexpr bool contains(const Key& key) const {
    return this->find(key) != end();
  }

  constexpr iterator lower_bound(const Key& key) {
    return this->make_iterator(this->lower_bound_key(key));
  }
  constexpr const_iterator lower_bound(const Key& key) const {
    return this->make_iterator(this->lower_bound_key(key));
  }

  ////////////
  // Observers

  constexpr const key_container_type& keys() const noexcept {
    return storage.keys;
  }

  constexpr const mapped_container_type& values() const noexcept {
    return storage.values;
  }

 private:
  constexpr std::pair<iterator, bool> insert_generic(const_key_iterator hint,
                                                     const value_type& value) {
    return this->insert_generic(hint, value.first,
                                std::forward<mapped_type>(value.second));
  }
  constexpr std::pair<iterator, bool> insert_generic(const_key_iterator hint,
                                                     value_type&& value) {
    return this->insert_generic(hint, value.first, std::move(value.second));
  }

  template <class K = key_type, class... Args>
  constexpr std::pair<iterator, bool> insert_generic(
      [[maybe_unused]] const_key_iterator hint, K&& key, Args&&... args) {
    // NOTE: We ignore the hint for now
    auto existing_it = this->lower_bound_key(key);
    if ((existing_it == storage.keys.cend()) || (*existing_it != key)) {
      auto existing_value_it = this->matching_value_iterator(existing_it);
      auto key_it = storage.keys.insert(existing_it, std::forward<K>(key));
      auto value_it = storage.values.emplace(existing_value_it,
                                             std::forward<Args>(args)...);
      return {this->make_iterator(key_it, value_it), true};
    } else {
      return {this->make_iterator(existing_it), false};
    }
  }

  constexpr key_iterator lower_bound_key(const Key& key) {
    return std::ranges::lower_bound(storage.keys, key, compare);
  }
  constexpr const_key_iterator lower_bound_key(const Key& key) const {
    return std::ranges::lower_bound(storage.keys, key, compare);
  }

  constexpr value_iterator matching_value_iterator(key_iterator key_it) {
    return std::begin(storage.values) +
           std::distance(std::begin(storage.keys), key_it);
  }
  constexpr const_value_iterator matching_value_iterator(
      const_key_iterator key_it) const {
    if (key_it == std::end(storage.keys)) {
      // Ideally we shouldn't have to check for end,
      // but it seems there's a segfault in Debug when calculating the distance
      return std::end(storage.values);
    } else {
      return std::begin(storage.values) +
             std::distance(std::begin(storage.keys), key_it);
    }
  }

 private:
  containers storage; // AKA c
  key_compare compare;
};

// https://stackoverflow.com/a/79004379
template <class FlatMap>
void reserve_map(FlatMap& map, typename FlatMap::size_type new_cap) {
  auto containers = map.extract();
  containers.keys.reserve(new_cap);
  containers.values.reserve(new_cap);
  map.replace(std::move(containers.keys), std::move(containers.values));
};
template <class FlatSet>
void reserve_set(FlatSet& set, typename FlatSet::size_type new_cap) {
  auto key_container = set.extract();
  key_container.reserve(new_cap);
  set.replace(std::move(key_container));
};

} // AOC_EXPORT_NAMESPACE(aoc)

#endif // AOC_FLAT_H
