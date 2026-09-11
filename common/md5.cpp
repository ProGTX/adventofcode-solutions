// C entry points over common/md5.h,
// which holds the implementation and the licence it came under.
// CMake builds this into the aoc_md5 static library,
// for the Rust solutions and for C++ outside Release

#include "md5.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <span>
#include <string_view>

namespace constant = aoc::constant;

extern "C" {

void aoc_md5(const void* message, std::size_t size, aoc::Digest* digest) {
  *digest = aoc::md5(std::string_view{static_cast<const char*>(message), size});
}

void aoc_md5_fixed(aoc::Block* blocks, std::size_t size, std::size_t count,
                   aoc::Digest* digests) {
  // A batch is a window onto the caller's blocks,
  // nothing is copied and nothing is reinterpreted
  auto index = 0uz;
  for (; (index + constant::md5_lanes) <= count; index += constant::md5_lanes) {
    const auto batch = std::span<aoc::Block, constant::md5_lanes>{
        blocks + index, constant::md5_lanes};
    std::ranges::copy(aoc::detail::md5_fixed(batch, size), digests + index);
  }
  // Whatever is left over goes through one lane at a time
  for (; index < count; ++index) {
    const auto batch = std::span<aoc::Block, 1>{blocks + index, 1};
    std::ranges::copy(aoc::detail::md5_fixed(batch, size), digests + index);
  }
}

void aoc_md5_many(const void* const* messages, const std::size_t* sizes,
                  std::size_t count, aoc::Digest* digests) {
  // Kept between calls:
  // zeroing these per call costs more than the hashing
  thread_local auto blocks = std::array<aoc::Block, constant::md5_lanes>{};
  thread_local auto blocked = 0uz;

  auto index = 0uz;
  while ((index + constant::md5_lanes) <= count) {
    // A batch has to be one block each and all the same size,
    // which is what lets the lanes run in step
    const auto size = sizes[index];
    auto uniform =
        (size + constant::md5_padding_size) <= constant::md5_block_size;
    for (auto lane = 1uz; uniform && (lane < constant::md5_lanes); ++lane) {
      uniform = sizes[index + lane] == size;
    }
    if (!uniform) {
      break;
    }

    // A message of the same size writes the same bytes
    // so the padding behind it stays valid
    if (size != blocked) {
      blocks = {};
      blocked = size;
    }
    for (auto lane = 0uz; lane < constant::md5_lanes; ++lane) {
      const auto* bytes =
          static_cast<const unsigned char*>(messages[index + lane]);
      std::copy_n(bytes, size, blocks[lane].begin());
    }

    std::ranges::copy(aoc::detail::md5_fixed<constant::md5_lanes>(blocks, size),
                      digests + index);
    index += constant::md5_lanes;
  }

  // Whatever is left over, or was never uniform in the first place
  for (; index < count; ++index) {
    aoc_md5(messages[index], sizes[index], digests + index);
  }
}

} // extern "C"
