fn parse(filename: &str) -> String {
    aoc::file::read_string(filename)
}

const MAX_OPERAND_DIGITS: usize = 3;

// A cursor over the input,
// in the spirit of the Kaleidoscope tutorial's token stream:
// `try*` methods attempt to consume a semantic element at the current position
// and rewind on failure so the caller can try something else instead
// (or just skip a character).
struct Cursor<'a> {
    bytes: &'a [u8],
    pos: usize,
}

impl<'a> Cursor<'a> {
    fn new(line: &'a str) -> Self {
        Self {
            bytes: line.as_bytes(),
            pos: 0,
        }
    }

    fn peek(&self) -> Option<u8> {
        self.bytes.get(self.pos).copied()
    }

    fn advance(&mut self) -> Option<u8> {
        let byte = self.peek()?;
        self.pos += 1;
        Some(byte)
    }

    fn try_consume_literal(&mut self, literal: &str) -> bool {
        let start = self.pos;
        for expected in literal.bytes() {
            if self.advance() != Some(expected) {
                self.pos = start;
                return false;
            }
        }
        return true;
    }

    fn try_parse_number(&mut self) -> Option<u32> {
        let start = self.pos;
        while self.pos - start < MAX_OPERAND_DIGITS
            && self.peek().is_some_and(|byte| byte.is_ascii_digit())
        {
            self.advance();
        }
        if self.pos == start {
            return None;
        }
        let digits = std::str::from_utf8(&self.bytes[start..self.pos]).unwrap();
        return Some(digits.parse().unwrap());
    }

    fn try_parse_mul(&mut self) -> Option<u32> {
        let start = self.pos;
        if self.try_consume_literal("mul(")
            && let Some(left_operand) = self.try_parse_number()
            && self.try_consume_literal(",")
            && let Some(right_operand) = self.try_parse_number()
            && self.try_consume_literal(")")
        {
            return Some(left_operand * right_operand);
        }
        self.pos = start;
        return None;
    }
}

fn parse_multiplications(line: &str) -> u32 {
    let mut cursor = Cursor::new(line);
    let mut sum = 0u32;
    while cursor.peek().is_some() {
        match cursor.try_parse_mul() {
            Some(product) => sum += product,
            None => {
                cursor.advance();
            }
        }
    }
    return sum;
}

fn parse_multiplications_with_enablers(line: &str) -> u32 {
    let mut cursor = Cursor::new(line);
    let mut sum = 0u32;
    let mut mul_enabled = true;
    while cursor.peek().is_some() {
        if cursor.try_consume_literal("do()") {
            mul_enabled = true;
        } else if cursor.try_consume_literal("don't()") {
            mul_enabled = false;
        } else if let Some(product) = cursor.try_parse_mul() {
            sum += product * u32::from(mul_enabled);
        } else {
            cursor.advance();
        }
    }
    return sum;
}

fn main() {
    println!("Asserts");
    assert_eq!(2024, parse_multiplications("mul(44,46)"));
    assert_eq!(
        33,
        parse_multiplications("xmul(2,4)&mul[3,7]!^don't()_mul(5,5)")
    );
    assert_eq!(
        1019406,
        parse_multiplications("[#from())when()/}+%mul(982,733)mul(700,428)}}")
    );
    assert_eq!(
        8,
        parse_multiplications_with_enablers("xmul(2,4)&mul[3,7]!^don't()_mul(5,5)")
    );

    println!("Part 1");
    let example = parse("day03.example");
    aoc::expect_result!(161, parse_multiplications(&example));
    let example2 = parse("day03.example2");
    aoc::expect_result!(161, parse_multiplications(&example2));
    let input = parse("day03.input");
    aoc::expect_result!(174960292, parse_multiplications(&input));

    println!("Part 2");
    aoc::expect_result!(161, parse_multiplications_with_enablers(&example));
    aoc::expect_result!(48, parse_multiplications_with_enablers(&example2));
    aoc::expect_result!(56275602, parse_multiplications_with_enablers(&input));
}
