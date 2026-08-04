module;

// compiler.h is macros only, safe to pull in ahead of the module declaration
// just to find out whether AOC_IMPORT_STD is set
#include "compiler.h"

#ifdef AOC_IMPORT_STD

// `import std` does not carry macros across
// and in a Debug build AOC_ASSERT expands to assert(),
// so this one header still has to be textual
#include <cassert>

#else

// TODO: This only seems to work with Clang, not with MSVC or GCC

// WORKAROUND: Our header files being exported also include these headers,
// leading to duplicate symbols -
// module linkage is not the same as global linkage.
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <charconv>
#include <chrono>
#include <cmath>
#include <compare>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <exception>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <print>
#include <ranges>
#include <set>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#endif // AOC_IMPORT_STD

export module aoc;

#ifdef AOC_IMPORT_STD
// With no std headers in the global module fragment
// there is nothing for the textual declarations in a consumer to clash with,
// which is what used to break this module on MSVC and GCC.
import std;
#endif

// hash.h names ankerl::unordered_dense in its exported aliases and hashes.
// Imported rather than included so the header does not land
// in the global module fragment of every consumer.
import ankerl.unordered_dense;

#include "algorithm.h"
#include "assert.h"
#include "bitmap_set.h"
#include "combinations.h"
#include "compiler.h"
#include "concepts.h"
#include "dijkstra.h"
#include "flat.h"
#include "functional.h"
#include "grid.h"
#include "hash.h"
#include "math.h"
#include "md5.h"
#include "parse.h"
#include "point.h"
#include "range_to.h"
#include "ranges.h"
#include "ratio.h"
#include "static_vector.h"
#include "string.h"
#include "timer.h"
#include "utility.h"
