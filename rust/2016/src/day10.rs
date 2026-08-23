use aoc::{closed_range::ClosedRange, iter::CollectArrayVec};
use arrayvec::ArrayVec;
use itertools::Itertools;

type InputBin = (u32, usize);

#[derive(Clone, Default)]
struct Bot {
    low: usize,
    high: usize,
}

const FIRST_OUTPUT_ID: usize = 1000;
const MAX_OUTPUTS: usize = 21;

type Input = (Vec<InputBin>, Vec<Bot>, usize);

fn parse_target(kind: &str, id: &str, num_outputs: &mut usize) -> usize {
    let id = id.parse::<usize>().unwrap();
    match kind {
        "bot" => id,
        "output" => {
            *num_outputs = (*num_outputs).max(id + 1);
            id + FIRST_OUTPUT_ID
        }
        _ => panic!("Invalid target: {kind}"),
    }
}

fn parse(filename: &str) -> Input {
    let mut input_bins = Vec::new();
    let mut bots = Vec::new();
    let mut num_outputs = 0;

    for line in aoc::file::read_lines(filename) {
        let words = line.split_whitespace().collect_vec();
        if words[0] == "value" {
            // value V goes to bot B
            let value = words[1].parse::<u32>().unwrap();
            let bot = words[5].parse::<usize>().unwrap();
            input_bins.push((value, bot));
        } else {
            // bot B gives low to <target> and high to <target>
            let id = words[1].parse::<usize>().unwrap();
            let low = parse_target(words[5], words[6], &mut num_outputs);
            let high = parse_target(words[10], words[11], &mut num_outputs);
            if bots.len() <= id {
                bots.resize_with(id + 1, Bot::default);
            }
            bots[id] = Bot { low, high };
        }
    }

    return (input_bins, bots, num_outputs);
}

fn solve_case<const MULTIPLY: bool>((input_bins, bots, num_outputs): &Input) -> u32 {
    let target = if (bots.len() <= 3) { (2, 5) } else { (17, 61) };
    let mut values = vec![ArrayVec::<u32, 2>::new(); bots.len()];
    for &(value, bot) in input_bins {
        values[bot].push(value);
    }
    let mut outputs = std::iter::repeat_n(0_u32, *num_outputs).collect_array_vec::<MAX_OUTPUTS>();

    let mut bot_stack = Vec::new();
    bot_stack.push(values.iter().position(|arr| arr.len() == 2).unwrap());
    while !bot_stack.is_empty() {
        let bot_id = bot_stack.pop().unwrap();
        let sorted = ClosedRange::new(values[bot_id][0], values[bot_id][1]);
        if (!MULTIPLY && (sorted.begin == target.0) && (sorted.end == target.1)) {
            return bot_id as u32;
        }
        let mut give = |other: usize, value: u32| {
            if (other >= FIRST_OUTPUT_ID) {
                outputs[other - FIRST_OUTPUT_ID] = value;
            } else {
                values[other].push(value);
                if (values[other].len() == 2) {
                    bot_stack.push(other);
                }
            }
        };
        give(bots[bot_id].low, sorted.begin);
        give(bots[bot_id].high, sorted.end);
        values[bot_id].clear();
    }

    if (MULTIPLY) {
        return outputs.into_iter().take(3).product();
    }

    unreachable!("Bot not found");
}

fn main() {
    println!("Part 1");
    let example = parse("day10.example");
    aoc::expect_result!(2, solve_case::<false>(&example));
    let input = parse("day10.input");
    aoc::expect_result!(116, solve_case::<false>(&input));

    println!("Part 2");
    aoc::expect_result!(30, solve_case::<true>(&example));
    aoc::expect_result!(23903, solve_case::<true>(&input));
}
