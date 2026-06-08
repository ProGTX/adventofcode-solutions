use std::{array, collections::HashSet};

use itertools::Itertools;

#[derive(Clone, Copy, Debug)]
enum Reg {
    W = 0,
    X = 1,
    Y = 2,
    Z = 3,
}

#[derive(Clone, Copy, Debug)]
enum Operand {
    Reg(Reg),
    Num(i64),
}

#[derive(Clone, Copy, Debug)]
enum Op {
    Add,
    Mul,
    Div,
    Mod,
    Eql,
    Inp,
}

#[derive(Clone, Copy, Debug)]
struct Instr {
    op: Op,
    dst: Reg,
    src: Operand,
}

fn parse_reg(s: &str) -> Reg {
    match s {
        "w" => Reg::W,
        "x" => Reg::X,
        "y" => Reg::Y,
        "z" => Reg::Z,
        _ => panic!("unknown register: {s}"),
    }
}

fn parse_program(lines: &[&str]) -> Vec<Instr> {
    lines
        .iter()
        .filter(|line| !line.is_empty())
        .map(|line| {
            let parts: Vec<&str> = line.split_whitespace().collect();
            let op = match parts[0] {
                "inp" => Op::Inp,
                "add" => Op::Add,
                "mul" => Op::Mul,
                "div" => Op::Div,
                "mod" => Op::Mod,
                "eql" => Op::Eql,
                _ => panic!("unknown op: {}", parts[0]),
            };
            let dst = parse_reg(parts[1]);
            let src = if matches!(op, Op::Inp) {
                Operand::Num(0)
            } else {
                let p2 = parts[2];
                match p2 {
                    "w" | "x" | "y" | "z" => Operand::Reg(parse_reg(p2)),
                    _ => Operand::Num(p2.parse().expect("Expected number")),
                }
            };
            Instr { op, dst, src }
        })
        .collect()
}

fn parse(filename: &str) -> Vec<Instr> {
    let lines = aoc::file::read_lines(filename);
    parse_program(&lines.iter().map(|s| s.as_str()).collect::<Vec<_>>())
}

type Registers = [i64; 4];

fn execute(mut regs: Registers, instructions: &[Instr], input: &[i64]) -> [i64; 4] {
    let mut input = input.iter().copied();
    for instr in instructions {
        let d = instr.dst as usize;
        let src = match instr.src {
            Operand::Reg(r) => regs[r as usize],
            Operand::Num(n) => n,
        };
        match instr.op {
            Op::Inp => regs[d] = input.next().expect("not enough input values"),
            Op::Add => regs[d] += src,
            Op::Mul => regs[d] *= src,
            Op::Div => regs[d] /= src,
            Op::Mod => regs[d] %= src,
            Op::Eql => regs[d] = (regs[d] == src) as i64,
        }
    }
    regs
}

fn to_number(digits: &[i64]) -> u64 {
    digits.iter().fold(0u64, |acc, &d| acc * 10 + d as u64)
}

const NUM_BLOCKS: usize = 14;
type ZOutputCache = [HashSet<i64>; NUM_BLOCKS];

fn solve_case<const SMALLEST: bool>(
    instructions: &[Instr],
    valid_z_output_cache: &mut Option<ZOutputCache>,
) -> u64 {
    // Split the instructions into blocks
    let starts: Vec<usize> = instructions
        .iter()
        .enumerate()
        .filter(|(_, instr)| matches!(instr.op, Op::Inp))
        .map(|(i, _)| i)
        .collect();
    let blocks: Vec<&[Instr]> = starts
        .windows(2)
        .map(|w| &instructions[w[0]..w[1]])
        .chain(std::iter::once(&instructions[*starts.last().unwrap()..]))
        .collect();

    // The following algorithm works based on this post:
    // https://www.reddit.com/r/adventofcode/comments/rnqabd/comment/hpu9wk3/

    let valid_z_output = if let Some(vzo) = valid_z_output_cache.as_ref() {
        vzo.clone()
    } else {
        let mut valid_z_output = array::from_fn(|_| HashSet::new());

        // This limit is somewhat arbitrary, it happens to work for my input
        // Could be slightly lower, but this is a nice number
        const Z_LIMIT: i64 = 314159;

        // Find valid z outputs for each block
        valid_z_output[NUM_BLOCKS - 1].insert(0);
        for (block_id, block) in blocks.iter().enumerate().rev() {
            let valid_z_input = (1..=9)
                .cartesian_product(0..Z_LIMIT)
                .filter_map(|(input, z)| {
                    let mut regs = Registers::default();
                    regs[Reg::Z as usize] = z;
                    let regs = execute(regs, &block, &[input]);
                    return if valid_z_output[block_id].contains(&regs[Reg::Z as usize]) {
                        Some(z)
                    } else {
                        None
                    };
                })
                .collect();
            if (block_id > 0) {
                valid_z_output[block_id - 1] = valid_z_input;
            }
        }
        *valid_z_output_cache = Some(valid_z_output);
        valid_z_output_cache.as_ref().unwrap().clone()
    };

    // For each block, find the first input that produces a valid z.
    // Carry over the computed z between blocks.
    let mut max_input = <[i64; NUM_BLOCKS]>::default();
    let mut z = 0;
    for (block_id, block) in blocks.iter().enumerate() {
        max_input[block_id] = (1..=9)
            .map(|input| if SMALLEST { input } else { 10 - input })
            .find(|input| {
                let mut regs = Registers::default();
                regs[Reg::Z as usize] = z;
                let regs = execute(regs, &block, &[*input]);
                return if valid_z_output[block_id].contains(&regs[Reg::Z as usize]) {
                    z = regs[Reg::Z as usize];
                    true
                } else {
                    false
                };
            })
            .unwrap();
    }

    return to_number(&max_input);
}

fn test(lines: &[&str], input: &[i64]) -> u64 {
    to_number(&execute(
        Registers::default(),
        &parse_program(&lines),
        &input,
    ))
}

fn main() {
    println!("Unit tests");
    aoc::expect_result!(200, test(&["inp x", "mul x -1"], &[-2]));
    aoc::expect_result!(
        901,
        test(&["inp z", "inp x", "mul z 3", "eql z x"], &[3, 9])
    );
    aoc::expect_result!(
        1101,
        test(
            &[
                "inp w", "add z w", "mod z 2", "div w 2", "add y w", "mod y 2", "div w 2",
                "add x w", "mod x 2", "div w 2", "mod w 2",
            ],
            &[13]
        )
    );

    // valid_z_output is shared between cases to speed up computation
    let mut valid_z_output = None;

    println!("Part 1");
    let input = parse("day24.input");
    aoc::expect_result!(
        99919765949498,
        solve_case::<false>(&input, &mut valid_z_output)
    );

    println!("Part 2");
    aoc::expect_result!(
        24913111616151,
        solve_case::<true>(&input, &mut valid_z_output)
    );
}
