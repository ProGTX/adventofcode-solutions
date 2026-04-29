// https://adventofcode.com/2021/day/16

#include "../common/common.h"
#include "../common/rust.h"

#include <iostream>
#include <limits>
#include <print>
#include <span>

auto parse(String const& filename) -> String {
  return aoc::read_file(filename);
}

fn parse_bits(str hex_packet) -> Vec<u8> {
  auto bits = Vec<u8>{};
  bits.reserve(hex_packet.size() * 4);
  for (char c : hex_packet) {
    u8 nibble = 0;
    if (c >= '0' && c <= '9') {
      nibble = static_cast<u8>(c - '0');
    } else if (c >= 'A' && c <= 'F') {
      nibble = static_cast<u8>(c - 'A' + 10);
    } else if (c >= 'a' && c <= 'f') {
      nibble = static_cast<u8>(c - 'a' + 10);
    } else {
      continue;
    }
    for (int i = 3; i >= 0; --i) {
      bits.push_back(static_cast<u8>((nibble >> i) & 1));
    }
  }
  return bits;
}

template <std::integral T>
fn bits_to_number(std::span<const u8> bits) -> T {
  T result = 0;
  for (u8 bit : bits) {
    result = static_cast<T>((result << 1) | static_cast<T>(bit));
  }
  return result;
}

struct ParseResult {
  usize packet_size;
  u64 value;
  u32 version_sum;
};

fn parse_packet(std::span<const u8> packet, usize num_packets, u8 operation)
    -> ParseResult {
  usize packet_size = 0;
  u32 version_sum = 0;
  auto value = [&]() -> Option<u64> {
    switch (operation) {
      case 1:
        return u64{1}; // product identity
      case 2:
        return std::numeric_limits<u64>::max(); // min identity
      case 5:
      case 6:
      case 7:
        return None; // comparison: lhs not yet seen
      default:
        return u64{0}; // sum/max identity
    }
  }();

  for (usize i = 0; i < num_packets; ++i) {
    if (packet.size() < 4) {
      return {packet_size, value.value(), version_sum};
    }
    version_sum += bits_to_number<u32>(packet.first(3));
    let type_id = bits_to_number<u8>(packet.subspan(3, 3));
    packet = packet.subspan(6);
    packet_size += 6;

    u64 rhs = 0;
    if (type_id == 4) {
      u64 literal_value = 0;
      loop {
        let group_bit = packet[0];
        let group_value = bits_to_number<u64>(packet.subspan(1, 4));
        packet = packet.subspan(5);
        packet_size += 5;
        literal_value = (literal_value << 4) | group_value;
        if (group_bit == 0) {
          break;
        }
      }
      rhs = literal_value;
    } else {
      let length_type_id = packet[0];
      ParseResult subpacket_result{};
      if (length_type_id == 0) {
        let subpacket_size = bits_to_number<usize>(packet.subspan(1, 15));
        packet = packet.subspan(16);
        packet_size += 16;
        subpacket_result =
            parse_packet(packet.first(subpacket_size),
                         std::numeric_limits<usize>::max(), type_id);
      } else {
        let num_subpackets = bits_to_number<usize>(packet.subspan(1, 11));
        packet = packet.subspan(12);
        packet_size += 12;
        subpacket_result = parse_packet(packet, num_subpackets, type_id);
      }
      version_sum += subpacket_result.version_sum;
      packet = packet.subspan(subpacket_result.packet_size);
      packet_size += subpacket_result.packet_size;
      rhs = subpacket_result.value;
    }

    value = [&]() -> u64 {
      switch (operation) {
        case 0:
          return value.value() + rhs;
        case 1:
          return value.value() * rhs;
        case 2:
          return std::min(value.value(), rhs);
        case 3:
          return std::max(value.value(), rhs);
        case 5:
          return value ? (value.value() > rhs ? u64{1} : u64{0}) : rhs;
        case 6:
          return value ? (value.value() < rhs ? u64{1} : u64{0}) : rhs;
        case 7:
          return value ? (value.value() == rhs ? u64{1} : u64{0}) : rhs;
        default:
          return rhs;
      }
    }();
  }

  return {packet_size, value.value(), version_sum};
}

fn solve_case1(str hex_packet) -> u32 {
  return parse_packet(parse_bits(hex_packet), 1, 0).version_sum;
}

fn solve_case2(str hex_packet) -> u64 {
  return parse_packet(parse_bits(hex_packet), 1, 0).value;
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(6u, solve_case1("D2FE28"));
  AOC_EXPECT_RESULT(9u, solve_case1("38006F45291200"));
  AOC_EXPECT_RESULT(16u, solve_case1("8A004A801A8002F478"));
  AOC_EXPECT_RESULT(12u, solve_case1("620080001611562C8802118E34"));
  AOC_EXPECT_RESULT(23u, solve_case1("C0015000016115A2E0802F182340"));
  AOC_EXPECT_RESULT(31u, solve_case1("A0016C880162017C3686B18A3D4780"));
  let example = parse("day16.example");
  AOC_EXPECT_RESULT(6u, solve_case1(example));
  let input = parse("day16.input");
  AOC_EXPECT_RESULT(925u, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(3ull, solve_case2("C200B40A82"));
  AOC_EXPECT_RESULT(54ull, solve_case2("04005AC33890"));
  AOC_EXPECT_RESULT(7ull, solve_case2("880086C3E88112"));
  AOC_EXPECT_RESULT(9ull, solve_case2("CE00C43D881120"));
  AOC_EXPECT_RESULT(1ull, solve_case2("D8005AC2A8F0"));
  AOC_EXPECT_RESULT(0ull, solve_case2("F600BC2D8F"));
  AOC_EXPECT_RESULT(0ull, solve_case2("9C005AC2F8F0"));
  AOC_EXPECT_RESULT(1ull, solve_case2("9C0141080250320F1802104A08"));
  AOC_EXPECT_RESULT(2021ull, solve_case2(example));
  AOC_EXPECT_RESULT(342997120375ull, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
