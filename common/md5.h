#ifndef AOC_MD5_H
#define AOC_MD5_H

// MD5, ported from the Rust implementation in
// https://github.com/maneatingape/advent-of-code-rust (src/util/md5.rs)
//
// MIT License
//
// Copyright (c) 2023-2026 maneatingape
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Changed from the original:
// the lane count is a template parameter driving the ordinary hash as well,
// so one lane is a plain MD5
// and several are messages hashed side by side, one per SIMD lane.
// And it is all constexpr.

#include "compiler.h"

#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#endif

AOC_EXPORT_NAMESPACE(aoc) {

namespace constant {

/// How many bytes a digest is
inline constexpr const auto md5_digest_size = 16uz;

/// How many messages a batch hashes side by side.
/// Eight 32-bit lanes is one AVX2 register and SSE2 splits it into two.
inline constexpr const auto md5_lanes = 8uz;

/// What a message shorter than 56 bytes pads out to
inline constexpr const auto md5_block_size = 64uz;

/// The terminator byte and the eight-byte message length in MD5 padding
inline constexpr const auto md5_padding_size = 1uz + sizeof(std::uint64_t);

/// How many messages one `aoc_md5_many` call takes at most
inline constexpr const auto md5_max_batch = 64uz;

} // namespace constant

/// The digest, in the order it is written out
using Digest = std::array<unsigned char, constant::md5_digest_size>;

/// A message padded out to its block
using Block = std::array<unsigned char, constant::md5_block_size>;

#ifdef AOC_MD5_LIBRARY
// The same code as below, compiled into the aoc_md5 library by md5.cpp
// C linkage, but the types are the ones above,
// so nothing has to be reinterpreted on either side
extern "C" {
void aoc_md5(const void* message, std::size_t size, Digest* digest);
void aoc_md5_fixed(Block* blocks, std::size_t size, std::size_t count,
                   Digest* digests);
void aoc_md5_many(const void* const* messages, const std::size_t* sizes,
                  std::size_t count, Digest* digests);
void aoc_md5_stretch(Digest* digests, std::size_t count,
                     std::size_t stretches);
}
#endif

namespace detail {

/// One 32-bit word of every lane's message, or of every lane's state
template <std::size_t Lanes>
using Words = std::array<std::uint32_t, Lanes>;

/// The state each lane starts from
template <std::size_t Lanes>
constexpr std::array<Words<Lanes>, 4> md5_initial_state() {
  auto state = std::array<Words<Lanes>, 4>{};
  state[0].fill(0x67452301);
  state[1].fill(0xefcdab89);
  state[2].fill(0x98badcfe);
  state[3].fill(0x10325476);
  return state;
}

/// Word `index` of each lane's block, read as little-endian.
template <std::size_t Lanes>
constexpr Words<Lanes> message(std::span<const Block, Lanes> blocks,
                               std::size_t index) {
  auto words = Words<Lanes>{};
  for (auto lane = 0uz; lane < Lanes; ++lane) {
    const auto& block = blocks[lane];
    words[lane] = std::uint32_t{block[4 * index]} |
                  (std::uint32_t{block[4 * index + 1]} << 8) |
                  (std::uint32_t{block[4 * index + 2]} << 16) |
                  (std::uint32_t{block[4 * index + 3]} << 24);
  }
  return words;
}

// The four rounds differ only in how they mix b, c and d
// Every lane does the same thing to its own words,
// which is the whole reason this vectorizes:
// no lane ever reads another lane's data

#define AOC_MD5_ROUND(name, mix)                                               \
  template <int SHIFT, std::size_t Lanes>                                      \
  constexpr void name(Words<Lanes>& a, const Words<Lanes>& b,                  \
                      const Words<Lanes>& c, const Words<Lanes>& d,            \
                      const Words<Lanes>& m, std::uint32_t constant) {         \
    for (auto lane = 0uz; lane < Lanes; ++lane) {                              \
      const auto f = mix;                                                      \
      a[lane] = b[lane] + std::rotl(f + a[lane] + constant + m[lane], SHIFT);  \
    }                                                                          \
  }

AOC_MD5_ROUND(round1, (b[lane] & c[lane]) | (~b[lane] & d[lane]))
AOC_MD5_ROUND(round2, (b[lane] & d[lane]) | (c[lane] & ~d[lane]))
AOC_MD5_ROUND(round3, b[lane] ^ c[lane] ^ d[lane])
AOC_MD5_ROUND(round4, c[lane] ^ (b[lane] | ~d[lane]))

#undef AOC_MD5_ROUND

/// Folds one block per lane into the running state.
template <std::size_t Lanes>
constexpr void transform(std::array<Words<Lanes>, 4>& state,
                         std::span<const Block, Lanes> blocks) {
  auto m = std::array<Words<Lanes>, 16>{};
  for (auto index = 0uz; index < m.size(); ++index) {
    m[index] = message(blocks, index);
  }

  auto a = state[0];
  auto b = state[1];
  auto c = state[2];
  auto d = state[3];

  // Round 1: f = (b & c) | (~b & d)
  round1<7>(a, b, c, d, m[0], 0xd76aa478);
  round1<12>(d, a, b, c, m[1], 0xe8c7b756);
  round1<17>(c, d, a, b, m[2], 0x242070db);
  round1<22>(b, c, d, a, m[3], 0xc1bdceee);
  round1<7>(a, b, c, d, m[4], 0xf57c0faf);
  round1<12>(d, a, b, c, m[5], 0x4787c62a);
  round1<17>(c, d, a, b, m[6], 0xa8304613);
  round1<22>(b, c, d, a, m[7], 0xfd469501);
  round1<7>(a, b, c, d, m[8], 0x698098d8);
  round1<12>(d, a, b, c, m[9], 0x8b44f7af);
  round1<17>(c, d, a, b, m[10], 0xffff5bb1);
  round1<22>(b, c, d, a, m[11], 0x895cd7be);
  round1<7>(a, b, c, d, m[12], 0x6b901122);
  round1<12>(d, a, b, c, m[13], 0xfd987193);
  round1<17>(c, d, a, b, m[14], 0xa679438e);
  round1<22>(b, c, d, a, m[15], 0x49b40821);

  // Round 2: f = (b & d) | (c & ~d)
  round2<5>(a, b, c, d, m[1], 0xf61e2562);
  round2<9>(d, a, b, c, m[6], 0xc040b340);
  round2<14>(c, d, a, b, m[11], 0x265e5a51);
  round2<20>(b, c, d, a, m[0], 0xe9b6c7aa);
  round2<5>(a, b, c, d, m[5], 0xd62f105d);
  round2<9>(d, a, b, c, m[10], 0x02441453);
  round2<14>(c, d, a, b, m[15], 0xd8a1e681);
  round2<20>(b, c, d, a, m[4], 0xe7d3fbc8);
  round2<5>(a, b, c, d, m[9], 0x21e1cde6);
  round2<9>(d, a, b, c, m[14], 0xc33707d6);
  round2<14>(c, d, a, b, m[3], 0xf4d50d87);
  round2<20>(b, c, d, a, m[8], 0x455a14ed);
  round2<5>(a, b, c, d, m[13], 0xa9e3e905);
  round2<9>(d, a, b, c, m[2], 0xfcefa3f8);
  round2<14>(c, d, a, b, m[7], 0x676f02d9);
  round2<20>(b, c, d, a, m[12], 0x8d2a4c8a);

  // Round 3: f = b ^ c ^ d
  round3<4>(a, b, c, d, m[5], 0xfffa3942);
  round3<11>(d, a, b, c, m[8], 0x8771f681);
  round3<16>(c, d, a, b, m[11], 0x6d9d6122);
  round3<23>(b, c, d, a, m[14], 0xfde5380c);
  round3<4>(a, b, c, d, m[1], 0xa4beea44);
  round3<11>(d, a, b, c, m[4], 0x4bdecfa9);
  round3<16>(c, d, a, b, m[7], 0xf6bb4b60);
  round3<23>(b, c, d, a, m[10], 0xbebfbc70);
  round3<4>(a, b, c, d, m[13], 0x289b7ec6);
  round3<11>(d, a, b, c, m[0], 0xeaa127fa);
  round3<16>(c, d, a, b, m[3], 0xd4ef3085);
  round3<23>(b, c, d, a, m[6], 0x04881d05);
  round3<4>(a, b, c, d, m[9], 0xd9d4d039);
  round3<11>(d, a, b, c, m[12], 0xe6db99e5);
  round3<16>(c, d, a, b, m[15], 0x1fa27cf8);
  round3<23>(b, c, d, a, m[2], 0xc4ac5665);

  // Round 4: f = c ^ (b | ~d)
  round4<6>(a, b, c, d, m[0], 0xf4292244);
  round4<10>(d, a, b, c, m[7], 0x432aff97);
  round4<15>(c, d, a, b, m[14], 0xab9423a7);
  round4<21>(b, c, d, a, m[5], 0xfc93a039);
  round4<6>(a, b, c, d, m[12], 0x655b59c3);
  round4<10>(d, a, b, c, m[3], 0x8f0ccc92);
  round4<15>(c, d, a, b, m[10], 0xffeff47d);
  round4<21>(b, c, d, a, m[1], 0x85845dd1);
  round4<6>(a, b, c, d, m[8], 0x6fa87e4f);
  round4<10>(d, a, b, c, m[15], 0xfe2ce6e0);
  round4<15>(c, d, a, b, m[6], 0xa3014314);
  round4<21>(b, c, d, a, m[13], 0x4e0811a1);
  round4<6>(a, b, c, d, m[4], 0xf7537e82);
  round4<10>(d, a, b, c, m[11], 0xbd3af235);
  round4<15>(c, d, a, b, m[2], 0x2ad7d2bb);
  round4<21>(b, c, d, a, m[9], 0xeb86d391);
  for (auto lane = 0uz; lane < Lanes; ++lane) {
    state[0][lane] += a[lane];
    state[1][lane] += b[lane];
    state[2][lane] += c[lane];
    state[3][lane] += d[lane];
  }
}

/// The state written out, one digest per lane
template <std::size_t Lanes>
constexpr std::array<Digest, Lanes> digests_of(
    const std::array<Words<Lanes>, 4>& state) {
  auto digests = std::array<Digest, Lanes>{};
  for (auto lane = 0uz; lane < Lanes; ++lane) {
    for (auto word = 0uz; word < 4; ++word) {
      const auto value = state[word][lane];
      digests[lane][4 * word + 0] = static_cast<unsigned char>(value);
      digests[lane][4 * word + 1] = static_cast<unsigned char>(value >> 8);
      digests[lane][4 * word + 2] = static_cast<unsigned char>(value >> 16);
      digests[lane][4 * word + 3] = static_cast<unsigned char>(value >> 24);
    }
  }
  return digests;
}

/// The 0x80 terminator and the bit count that pad a message out to its
/// last block,
/// which for a message under 56 bytes is its only block
constexpr void pad(Block& block, std::size_t size, std::uint64_t total_bits) {
  block[size] = 0x80;
  for (auto byte = 0uz; byte < 8; ++byte) {
    block[constant::md5_block_size - 8 + byte] =
        static_cast<unsigned char>(total_bits >> (8 * byte));
  }
}

/// Hashes one block per lane, in place
template <std::size_t Lanes>
constexpr std::array<Digest, Lanes> md5_fixed(std::span<Block, Lanes> blocks,
                                              std::size_t size) {
  for (auto& block : blocks) {
    detail::pad(block, size, 8 * size);
  }
  auto state = detail::md5_initial_state<Lanes>();
  detail::transform<Lanes>(state, blocks);
  return detail::digests_of(state);
}

/// Hashes one message of any length
constexpr Digest md5(std::string_view input) {
  const auto size = input.size();
  auto state = detail::md5_initial_state<1>();
  auto blocks = std::array<Block, 1>{};

  // The blocks the message fills on its own
  auto offset = 0uz;
  for (; size - offset >= constant::md5_block_size;
       offset += constant::md5_block_size) {
    for (auto byte = 0uz; byte < constant::md5_block_size; ++byte) {
      blocks[0][byte] = static_cast<unsigned char>(input[offset + byte]);
    }
    detail::transform<1>(state, blocks);
  }

  // What is left needs the terminator right after it
  // and the bit count at the very end,
  // which takes a second block when the two do not fit together
  const auto remaining = size - offset;
  blocks[0] = Block{};
  for (auto byte = 0uz; byte < remaining; ++byte) {
    blocks[0][byte] = static_cast<unsigned char>(input[offset + byte]);
  }
  blocks[0][remaining] = 0x80;
  const auto total_bits = std::uint64_t{size} * 8;
  const auto spilled =
      remaining + constant::md5_padding_size > constant::md5_block_size;
  if (!spilled) {
    for (auto byte = 0uz; byte < 8; ++byte) {
      blocks[0][constant::md5_block_size - 8 + byte] =
          static_cast<unsigned char>(total_bits >> (8 * byte));
    }
  }
  detail::transform<1>(state, blocks);

  if (spilled) {
    // The count alone, in a block of its own
    blocks[0] = Block{};
    for (auto byte = 0uz; byte < 8; ++byte) {
      blocks[0][constant::md5_block_size - 8 + byte] =
          static_cast<unsigned char>(total_bits >> (8 * byte));
    }
    detail::transform<1>(state, blocks);
  }

  return detail::digests_of(state)[0];
}

/// Hashes independent messages, batching the ones that can share lanes
constexpr void md5_many(std::span<const std::string_view> messages,
                        std::span<Digest> digests) {
  auto batched = 0uz;
  if (!messages.empty()) {
    const auto size = messages[0].size();
    const auto uniform =
        ((size + constant::md5_padding_size) <= constant::md5_block_size) &&
        std::all_of(messages.begin(), messages.end(),
                    [size](std::string_view message) {
                      return message.size() == size;
                    });
    if (uniform) {
      auto blocks = std::array<Block, constant::md5_lanes>{};
      for (; batched + constant::md5_lanes <= messages.size();
           batched += constant::md5_lanes) {
        for (auto lane = 0uz; lane < constant::md5_lanes; ++lane) {
          const auto message = messages[batched + lane];
          for (auto byte = 0uz; byte < size; ++byte) {
            blocks[lane][byte] = static_cast<unsigned char>(message[byte]);
          }
        }
        const auto hashed = md5_fixed<constant::md5_lanes>(blocks, size);
        std::copy_n(hashed.begin(), constant::md5_lanes,
                    digests.begin() + batched);
      }
    }
  }
  for (auto index = batched; index < messages.size(); ++index) {
    digests[index] = md5(messages[index]);
  }
}

/// Rehashes every digest as the 32 hex digits it is written out as,
/// `stretches` times over
constexpr void md5_stretch(std::span<Digest> digests, std::size_t stretches) {
  constexpr auto hex = std::string_view{"0123456789abcdef"};
  constexpr auto size = 2 * constant::md5_digest_size;
  for (auto first = 0uz; first < digests.size();
       first += constant::md5_lanes) {
    const auto count = std::min(constant::md5_lanes, digests.size() - first);
    // Every stretch rewrites the same 32 bytes,
    // so the padding behind them is written once and stays valid
    auto blocks = std::array<Block, constant::md5_lanes>{};
    for (auto step = 0uz; step < stretches; ++step) {
      for (auto lane = 0uz; lane < count; ++lane) {
        const auto& digest = digests[first + lane];
        // A byte at a time, so each nibble costs a shift and a mask
        for (auto index = 0uz; index < constant::md5_digest_size; ++index) {
          const auto byte = digest[index];
          blocks[lane][2 * index] = static_cast<unsigned char>(hex[byte >> 4]);
          blocks[lane][(2 * index) + 1] =
              static_cast<unsigned char>(hex[byte & 0xf]);
        }
      }
      const auto hashed = md5_fixed<constant::md5_lanes>(blocks, size);
      for (auto lane = 0uz; lane < count; ++lane) {
        digests[first + lane] = hashed[lane];
      }
    }
  }
}

} // namespace detail

/// Hashes one message of any length
constexpr Digest md5(std::string_view input) {
#ifdef AOC_MD5_LIBRARY
  // The library carries its own compile options
  // Only at run time: a constant expression has no library to call
  if !consteval {
    auto digest = Digest{};
    aoc_md5(input.data(), input.size(), &digest);
    return digest;
  }
#endif
  return detail::md5(input);
}

/// Hashes one block per lane, in place:
/// every block holds a message of `size` bytes and zeroes after it,
/// which is what a loop reusing them maintains.
/// Nothing is copied, so this is the one to reach for in a hot loop.
template <std::size_t Lanes>
constexpr std::array<Digest, Lanes> md5_fixed(std::array<Block, Lanes>& blocks,
                                              std::size_t size) {
#ifdef AOC_MD5_LIBRARY
  if !consteval {
    auto digests = std::array<Digest, Lanes>{};
    aoc_md5_fixed(blocks.data(), size, Lanes, digests.data());
    return digests;
  }
#endif
  return detail::md5_fixed<Lanes>(blocks, size);
}

/// Hashes messages that do not depend on each other.
/// Same-size messages that fit a single block go through the lanes together,
/// anything else falls back to hashing them one at a time.
constexpr void md5_many(std::span<const std::string_view> messages,
                        std::span<Digest> digests) {
#ifdef AOC_MD5_LIBRARY
  if !consteval {
    for (auto first = 0uz; first < messages.size();
         first += constant::md5_max_batch) {
      const auto batch =
          std::min(constant::md5_max_batch, messages.size() - first);
      auto pointers = std::array<const void*, constant::md5_max_batch>{};
      auto sizes = std::array<std::size_t, constant::md5_max_batch>{};
      for (auto index = 0uz; index < batch; ++index) {
        pointers[index] = messages[first + index].data();
        sizes[index] = messages[first + index].size();
      }
      aoc_md5_many(pointers.data(), sizes.data(), batch, &digests[first]);
    }
    return;
  }
#endif
  detail::md5_many(messages, digests);
}

/// Rehashes every digest as the 32 hex digits it is written out as,
/// `stretches` times over, in place.
/// The whole run stays in here, so the lanes are only written out once:
/// this is the one to reach for when a hash is stretched many times over
constexpr void md5_stretch(std::span<Digest> digests, std::size_t stretches) {
#ifdef AOC_MD5_LIBRARY
  if !consteval {
    aoc_md5_stretch(digests.data(), digests.size(), stretches);
    return;
  }
#endif
  detail::md5_stretch(digests, stretches);
}

static_assert(md5("abc") == Digest{0x90, 0x01, 0x50, 0x98, 0x3c, 0xd2, 0x4f,
                                   0xb0, 0xd6, 0x96, 0x3f, 0x7d, 0x28, 0xe1,
                                   0x7f, 0x72});

static_assert([] {
  auto digests = std::array<Digest, 1>{md5("abc")};
  detail::md5_stretch(digests, 1);
  return digests[0];
}() == Digest{0xec, 0x04, 0x05, 0xc5, 0xae, 0xf9, 0x3e, 0x77, 0x1c, 0xd8, 0x0e,
              0x0d, 0xb1, 0x80, 0xb8, 0x8b});

} // AOC_EXPORT_NAMESPACE(aoc)

#endif // AOC_MD5_H
