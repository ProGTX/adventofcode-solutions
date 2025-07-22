#ifndef AOC_COMPILER_H
#define AOC_COMPILER_H

#ifndef AOC_EXPORT
#define AOC_EXPORT
#endif

#if defined(_MSC_VER)
#define AOC_COMPILER_MSVC _MSC_VER
#elif defined(__GNUC__)
#if defined(__clang__)
#define AOC_COMPILER_CLANG __clang_major__
#else
#define AOC_COMPILER_GCC __GNUC__
#endif
#endif

#define AOC_NOT_CONSTEXPR(statement)                                           \
  if (!std::is_constant_evaluated()) {                                         \
    statement;                                                                 \
  }

namespace aoc {
template <int cpp_version>
constexpr bool not_constant_before() noexcept {
  return AOC_CXX_STANDARD < cpp_version;
}
} // namespace aoc

#endif // AOC_COMPILER_H
