use std::collections::HashSet;

type Numbers = Vec<u32>;
type Board = aoc::grid::Grid<u32>;

fn parse(filename: &str) -> (Numbers, Vec<Board>) {
    let lines = aoc::file::read_string(filename)
        .trim()
        .split("\n\n")
        .map(|line| line.to_string())
        .collect::<Vec<_>>();
    let mut line_it = lines.iter();
    let numbers = line_it
        .next()
        .unwrap()
        .split(',')
        .map(|num| num.parse::<u32>().unwrap())
        .collect::<Vec<_>>();
    let boards = line_it
        .map(|board_str| {
            let mut data = Vec::<u32>::new();
            for line in board_str.lines() {
                data.extend(
                    line.split_ascii_whitespace()
                        .map(|num| num.parse::<u32>().unwrap()),
                );
            }
            Board {
                data: data,
                num_rows: 5,
                num_columns: 5,
            }
        })
        .collect::<Vec<_>>();
    return (numbers, boards);
}

fn check_bingo(unmarked: &Board) -> bool {
    (0..5)
        .into_iter()
        .map(|start| {
            (unmarked.row(start).iter().sum::<u32>() == 0)
                || (unmarked.column(start).sum::<u32>() == 0)
        })
        .any(|b| b)
}

fn solve_case<const ALL_ROUNDS: bool>((numbers, boards): &(Numbers, Vec<Board>)) -> u32 {
    let mut winners = HashSet::new();
    let mut last = (0, 0);
    let mut unmarked_boards = boards.clone();
    'outer: for &number in numbers {
        for ((board_id, board), unmarked) in
            boards.iter().enumerate().zip(unmarked_boards.iter_mut())
        {
            let index = board.data.iter().position(|&n| n == number);
            if (index.is_none()) {
                continue;
            }
            unmarked.data[index.unwrap()] = 0;
            if (!check_bingo(unmarked)) {
                continue;
            } else if (false
                || !ALL_ROUNDS
                || (winners.insert(board_id) && (winners.len() == boards.len())))
            {
                last = (number, board_id);
                break 'outer;
            }
        }
    }
    return last.0 * unmarked_boards[last.1].data.iter().sum::<u32>();
}

fn main() {
    println!("Part 1");
    let example = parse("day04.example");
    assert_eq!(4512, solve_case::<false>(&example));
    let input = parse("day04.input");
    assert_eq!(64084, solve_case::<false>(&input));

    println!("Part 2");
    assert_eq!(1924, solve_case::<true>(&example));
    assert_eq!(12833, solve_case::<true>(&input));
}
