fn parse(filename: &str) -> String {
    aoc::file::read_string(&filename)
}

fn parse_bits(hex_packet: &str) -> Vec<u8> {
    let mut bits = Vec::with_capacity(hex_packet.len() * 4);
    for c in hex_packet.chars() {
        if let Some(n) = c.to_digit(16) {
            for i in (0..4).rev() {
                bits.push(((n >> i) & 1) as u8);
            }
        }
    }
    bits
}

struct ParseResult {
    packet_size: usize,
    value: u64,
    version_sum: u32,
}

fn parse_packet(mut packet: &[u8], num_packets: usize, operation: u8) -> ParseResult {
    let mut packet_size = 0;
    let mut version_sum = 0;
    let mut value = match operation {
        1 => Some(1),        // product
        2 => Some(u64::MAX), // min
        5 | 6 | 7 => None,   // comparison
        _ => Some(0),
    };
    for _ in 0..num_packets {
        if (packet.len() < 4) {
            return ParseResult {
                packet_size,
                value: value.unwrap(),
                version_sum,
            };
        }
        let version = aoc::math::binary_to_number::<u32>(&packet[0..3]);
        version_sum += version;
        let type_id: u8 = aoc::math::binary_to_number(&packet[3..6]);
        packet = &packet[6..];
        packet_size += 6;

        let rhs = match type_id {
            4 => {
                // Literal value
                let mut literal_value = 0;
                loop {
                    let group_bit = packet[0];
                    let group_value: u64 = aoc::math::binary_to_number(&packet[1..5]);
                    packet = &packet[5..];
                    packet_size += 5;

                    literal_value = (literal_value << 4) | group_value;
                    if (group_bit == 0) {
                        break;
                    }
                }

                literal_value
            }
            _ => {
                // Operator
                let length_type_id = packet[0];
                let subpacket_result = if (length_type_id == 0) {
                    // We know the length of all subpackets, but not the number
                    let subpacket_size: usize = aoc::math::binary_to_number(&packet[1..16]);
                    packet = &packet[16..];
                    packet_size += 16;

                    parse_packet(&packet[..subpacket_size], usize::MAX, type_id)
                } else {
                    // We know the number of subpackets, but not the length
                    let num_subpackets: usize = aoc::math::binary_to_number(&packet[1..12]);
                    packet = &packet[12..];
                    packet_size += 12;

                    parse_packet(&packet, num_subpackets, type_id)
                };

                version_sum += subpacket_result.version_sum;
                let subpacket_size = subpacket_result.packet_size;
                packet = &packet[subpacket_size..];
                packet_size += subpacket_size;

                subpacket_result.value
            }
        };

        value = Some(match operation {
            0 => value.unwrap() + rhs,
            1 => value.unwrap() * rhs,
            2 => value.unwrap().min(rhs),
            3 => value.unwrap().max(rhs),
            5 => {
                if let Some(lhs) = value {
                    if (lhs > rhs) { 1 } else { 0 }
                } else {
                    rhs
                }
            }
            6 => {
                if let Some(lhs) = value {
                    if (lhs < rhs) { 1 } else { 0 }
                } else {
                    rhs
                }
            }
            7 => {
                if let Some(lhs) = value {
                    if (lhs == rhs) { 1 } else { 0 }
                } else {
                    rhs
                }
            }
            _ => rhs,
        });
    }

    return ParseResult {
        packet_size,
        value: value.unwrap(),
        version_sum,
    };
}

fn solve_case1(hex_packet: &str) -> u32 {
    parse_packet(parse_bits(hex_packet).as_slice(), 1, 0).version_sum
}

fn solve_case2(hex_packet: &str) -> u64 {
    parse_packet(parse_bits(hex_packet).as_slice(), 1, 0).value
}

fn main() {
    println!("Part 1");

    aoc::expect_result!(6, solve_case1("D2FE28"));
    aoc::expect_result!(9, solve_case1("38006F45291200"));
    aoc::expect_result!(16, solve_case1("8A004A801A8002F478"));
    aoc::expect_result!(12, solve_case1("620080001611562C8802118E34"));
    aoc::expect_result!(23, solve_case1("C0015000016115A2E0802F182340"));
    aoc::expect_result!(31, solve_case1("A0016C880162017C3686B18A3D4780"));

    let example = parse("day16.example");
    aoc::expect_result!(6, solve_case1(&example));
    let input = parse("day16.input");
    aoc::expect_result!(925, solve_case1(&input));

    println!("Part 2");

    aoc::expect_result!(3, solve_case2("C200B40A82"));
    aoc::expect_result!(54, solve_case2("04005AC33890"));
    aoc::expect_result!(7, solve_case2("880086C3E88112"));
    aoc::expect_result!(9, solve_case2("CE00C43D881120"));
    aoc::expect_result!(1, solve_case2("D8005AC2A8F0"));
    aoc::expect_result!(0, solve_case2("F600BC2D8F"));
    aoc::expect_result!(0, solve_case2("9C005AC2F8F0"));
    aoc::expect_result!(1, solve_case2("9C0141080250320F1802104A08"));

    aoc::expect_result!(2021, solve_case2(&example));
    aoc::expect_result!(342997120375, solve_case2(&input));
}
