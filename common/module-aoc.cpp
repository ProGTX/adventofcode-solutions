module;

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
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <ostream>
#include <ranges>
#include <set>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

export module aoc;

export {

// Export all of our headers.
// This exports all the standard headers that are included transitively,
// hence the need for the workaround above.
// WORKAROUND: MSVC doesn't support CRTP with modules,
// add graph.h once that's fixed
#include "algorithm.h"
#include "assert.h"
#include "compiler.h"
#include "concepts.h"
#include "flat.h"
#include "grid.h"
#include "math.h"
#include "point.h"
#include "print.h"
#include "range_to.h"
#include "ranges.h"
#include "ratio.h"
#include "static_vector.h"
#include "string.h"
#include "timer.h"
#include "utility.h"
}
