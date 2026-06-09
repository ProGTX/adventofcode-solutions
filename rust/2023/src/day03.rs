fn parse(filename: &str) -> Vec<String> {
    aoc::file::read_lines(filename)
}

fn is_symbol(c: char) -> bool {
    !c.is_ascii_digit() && c != '.'
}

#[derive(Clone, Copy, PartialEq, Eq)]
struct Number {
    row: i32,
    column: i32,
    size: i32,
}

fn find_number(line: &str, linenum: i32, init_pos: i32) -> Option<Number> {
    let bytes = line.as_bytes();
    if !bytes[init_pos as usize].is_ascii_digit() {
        return None;
    }
    let mut start_pos = init_pos;
    while start_pos >= 0 && bytes[start_pos as usize].is_ascii_digit() {
        start_pos -= 1;
    }
    let column = start_pos + 1;
    let mut end_pos = init_pos + 1;
    while (end_pos as usize) < bytes.len() && bytes[end_pos as usize].is_ascii_digit() {
        end_pos += 1;
    }
    Some(Number {
        row: linenum - 1,
        column,
        size: end_pos - column,
    })
}

fn add_number<const CHECK_GEARS: bool>(
    line: &str,
    linenum: i32,
    pos: i32,
    all_numbers: &mut Vec<(Number, i32)>,
    current_numbers: &mut Vec<(Number, i32)>,
) {
    let Some(number) = find_number(line, linenum, pos) else {
        return;
    };
    if let Some(existing) = all_numbers.iter().find(|(n, _)| *n == number).copied() {
        if CHECK_GEARS && !current_numbers.iter().any(|(n, _)| *n == number) {
            current_numbers.push(existing);
        }
        return;
    }
    let number_value: i32 = line[number.column as usize..(number.column + number.size) as usize]
        .parse()
        .unwrap();
    all_numbers.push((number, number_value));
    if CHECK_GEARS && !current_numbers.iter().any(|(n, _)| *n == number) {
        current_numbers.push((number, number_value));
    }
}

fn solve_case<const CHECK_GEARS: bool>(lines: &[String]) -> i32 {
    let mut all_numbers: Vec<(Number, i32)> = Vec::new();
    let mut current_numbers: Vec<(Number, i32)> = Vec::new();
    let mut gear_ratios: Vec<i32> = Vec::new();
    let mut line_minus2 = String::new();
    let mut line_minus1 = String::new();

    for (linenum_idx, line) in lines.iter().enumerate() {
        let linenum = linenum_idx as i32 + 1;
        if linenum < 3 {
            if linenum == 1 {
                line_minus2 = line.clone();
            } else {
                line_minus1 = line.clone();
            }
            continue;
        }

        // Edge columns never contain a symbol, so inner bounds suffice
        for pos in 1..line_minus1.len() as i32 - 1 {
            let c = line_minus1.as_bytes()[pos as usize] as char;
            if !is_symbol(c) {
                continue;
            }
            if CHECK_GEARS {
                current_numbers.clear();
            }
            add_number::<CHECK_GEARS>(
                &line_minus2,
                linenum - 2,
                pos - 1,
                &mut all_numbers,
                &mut current_numbers,
            );
            add_number::<CHECK_GEARS>(
                &line_minus2,
                linenum - 2,
                pos,
                &mut all_numbers,
                &mut current_numbers,
            );
            add_number::<CHECK_GEARS>(
                &line_minus2,
                linenum - 2,
                pos + 1,
                &mut all_numbers,
                &mut current_numbers,
            );
            add_number::<CHECK_GEARS>(
                &line_minus1,
                linenum - 1,
                pos - 1,
                &mut all_numbers,
                &mut current_numbers,
            );
            add_number::<CHECK_GEARS>(
                &line_minus1,
                linenum - 1,
                pos + 1,
                &mut all_numbers,
                &mut current_numbers,
            );
            add_number::<CHECK_GEARS>(
                line,
                linenum,
                pos - 1,
                &mut all_numbers,
                &mut current_numbers,
            );
            add_number::<CHECK_GEARS>(
                //
                line,
                linenum,
                pos,
                &mut all_numbers,
                &mut current_numbers,
            );
            add_number::<CHECK_GEARS>(
                line,
                linenum,
                pos + 1,
                &mut all_numbers,
                &mut current_numbers,
            );
            if CHECK_GEARS && current_numbers.len() == 2 {
                gear_ratios.push(current_numbers[0].1 * current_numbers[1].1);
            }
        }
        line_minus2 = line.clone();
        std::mem::swap(&mut line_minus1, &mut line_minus2);
    }

    if !CHECK_GEARS {
        all_numbers.iter().map(|(_, v)| *v).sum()
    } else {
        gear_ratios.iter().sum()
    }
}

fn main() {
    println!("Part 1");
    let example = parse("day03.example");
    aoc::expect_result!(4361, solve_case::<false>(&example));
    let input = parse("day03.input");
    aoc::expect_result!(537732, solve_case::<false>(&input));

    println!("Part 2");
    aoc::expect_result!(467835, solve_case::<true>(&example));
    aoc::expect_result!(84883664, solve_case::<true>(&input));
}
