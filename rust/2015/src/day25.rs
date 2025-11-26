type Coords = (u32, u32);

fn parse(filename: &str) -> Coords {
    let file = aoc::file::read_string(&filename)
        .trim()
        .strip_suffix('.')
        .unwrap()
        .to_string();
    let (row, column) = file
        .split_once("row ")
        .unwrap()
        .1
        .split_once(", column ")
        .unwrap();
    (column.parse().unwrap(), row.parse().unwrap())
}

struct Code {
    value: u64,
    row: u32,
    column: u32,
}

fn next(code: Code) -> Code {
    let mut new_code = Code {
        value: code.value * 252533 % 33554393,
        row: code.row - 1,
        column: code.column + 1,
    };
    if (new_code.row == 0) {
        new_code.row = new_code.column;
        new_code.column = 1;
    }
    new_code
}

fn solve_case((column, row): Coords) -> u64 {
    let mut current = Code {
        value: 33511524,
        row: 1,
        column: 6,
    };
    while ((current.row != row) || (current.column != column)) {
        current = next(current);
    }
    current.value
}

fn main() {
    println!("Part 1");
    let example = parse("day25.example");
    assert_eq!(12231762, solve_case(example));
    let input = parse("day25.input");
    assert_eq!(2650453, solve_case(input));
}
