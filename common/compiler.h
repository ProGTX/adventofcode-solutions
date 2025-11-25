#ifndef AOC_COMPILER_H
#define AOC_COMPILER_H

#if defined(__INTELLISENSE__) && !defined(AOC_DISABLE_MODULES)
#define AOC_DISABLE_MODULES
#endif

#if defined(AOC_MODULE_SUPPORT) && defined(AOC_DISABLE_MODULES)
#undef AOC_MODULE_SUPPORT
#endif

#ifndef AOC_EXPORT
#define AOC_EXPORT
#elifdef AOC_DISABLE_MODULES
#undef AOC_EXPORT
#define AOC_EXPORT
#endif

#ifndef AOC_EXPORT_NAMESPACE
#define AOC_EXPORT_NAMESPACE(name) AOC_EXPORT namespace name
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

#endif // AOC_COMPILER_H
