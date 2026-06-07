#ifndef AOC_MD5_H
#define AOC_MD5_H

#include "compiler.h"

#include <array>
#include <string_view>

struct MD5_CTX {
  unsigned int state[4];
  unsigned int count[2];
  unsigned char buffer[64];
};
extern "C" {
void MD5Init(MD5_CTX*);
void MD5Update(MD5_CTX*, unsigned char*, unsigned int);
void MD5Final(unsigned char*, MD5_CTX*);
}

AOC_EXPORT_NAMESPACE(aoc) {

inline std::array<unsigned char, 16> md5(std::string_view input) {
  MD5_CTX ctx{};
  MD5Init(&ctx);
  MD5Update(&ctx,
            reinterpret_cast<unsigned char*>(const_cast<char*>(input.data())),
            static_cast<unsigned int>(input.size()));
  auto digest = std::array<unsigned char, 16>{};
  MD5Final(digest.data(), &ctx);
  return digest;
}

} // AOC_EXPORT_NAMESPACE(aoc)

#endif // AOC_MD5_H
