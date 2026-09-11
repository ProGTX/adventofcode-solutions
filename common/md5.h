#ifndef AOC_MD5_H
#define AOC_MD5_H

#include "compiler.h"
#include "utility.h"

#ifndef AOC_IMPORT_STD
#include <array>
#include <cstddef>
#include <string_view>
#endif

#ifdef AOC_HAVE_MD5
// OpenSSL's one-shot MD5, declared here rather than included,
// so that nothing else has to know where the OpenSSL headers are
extern "C" unsigned char* MD5(const unsigned char* data, std::size_t size,
                              unsigned char* digest);
#endif

AOC_EXPORT_NAMESPACE(aoc) {

inline std::array<unsigned char, 16> md5(std::string_view input) {
  auto digest = std::array<unsigned char, 16>{};
#ifdef AOC_HAVE_MD5
  MD5(reinterpret_cast<const unsigned char*>(input.data()), input.size(),
      digest.data());
#else
  // Built without OpenSSL, so there is nothing to hash with:
  // whichever solution asked for a hash skips right here
  static_cast<void>(input);
  return_incomplete();
#endif
  return digest;
}

} // AOC_EXPORT_NAMESPACE(aoc)

#endif // AOC_MD5_H
