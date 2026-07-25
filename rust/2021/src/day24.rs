use aoc::bitmap_set::BitmapSet;
use std::array;
use std::thread;

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

// Runs one block for BATCH different z values at once.
// Decoding an instruction is by far the dominant cost of the interpreter,
// and this shares it across the lanes.
// The lanes are independent of each other, so they also vectorize.
const BATCH: usize = 16;
type RegisterBatch = [[i64; BATCH]; 4];

fn execute_batch(regs: &mut RegisterBatch, instructions: &[Instr], input: i64) {
    for instr in instructions {
        let src: [i64; BATCH] = match instr.src {
            Operand::Reg(r) => regs[r as usize],
            Operand::Num(n) => [n; BATCH],
        };
        let dst = &mut regs[instr.dst as usize];
        match instr.op {
            Op::Inp => *dst = [input; BATCH],
            Op::Add => {
                for i in 0..BATCH {
                    dst[i] += src[i];
                }
            }
            Op::Mul => {
                for i in 0..BATCH {
                    dst[i] *= src[i];
                }
            }
            Op::Div => {
                for i in 0..BATCH {
                    dst[i] /= src[i];
                }
            }
            Op::Mod => {
                for i in 0..BATCH {
                    dst[i] %= src[i];
                }
            }
            Op::Eql => {
                for i in 0..BATCH {
                    dst[i] = (dst[i] == src[i]) as i64;
                }
            }
        }
    }
}

fn to_number(digits: &[i64]) -> u64 {
    digits.iter().fold(0u64, |acc, &d| acc * 10 + d as u64)
}

const NUM_BLOCKS: usize = 14;

// This limit is somewhat arbitrary, it happens to work for my input
// Could be slightly lower, but this is a nice number
const Z_LIMIT: i64 = 314159;

/// The lookup cost is what matters here, not the memory.
/// Every one of the ~40M block runs probes a set,
/// but only ~15k values ever get inserted,
/// and most of those lookups find nothing,
/// so a bitmap beats hashing by a wide margin.
/// Block outputs routinely overshoot the range,
/// which `contains` reports as absent.
/// Merging is a plain OR over the words,
/// so the per-thread results are cheap to combine.
///
/// The set costs Z_LIMIT/8 bytes no matter how few values it holds,
/// and these sets are sparse: the busiest block keeps 10k out of 314k.
type ZSet = BitmapSet<i64, { Z_LIMIT as usize }>;
type ZOutputCache = [ZSet; NUM_BLOCKS];

// Every z value for one block is independent of every other,
// so the range is split across threads
// Only the blocks themselves have to stay ordered
// since each one consumes the set produced by the block after it
fn valid_z_inputs(block: &[Instr], valid_output: &ZSet) -> ZSet {
    const MAX_PARALLELISM: usize = 16;
    let num_threads = thread::available_parallelism()
        .map_or(1, |n| n.get())
        .min(MAX_PARALLELISM);
    // A chunk covers whole batches, so the threads write disjoint z values
    let chunk = (Z_LIMIT as usize)
        .div_ceil(num_threads)
        .next_multiple_of(BATCH);

    thread::scope(|scope| {
        (0..Z_LIMIT)
            .step_by(chunk)
            .map(|z_start| {
                scope.spawn(move || {
                    let z_end = (z_start + chunk as i64).min(Z_LIMIT);
                    let mut valid_z_input = ZSet::new();
                    for input in 1..=9 {
                        for z_base in (z_start..z_end).step_by(BATCH) {
                            let mut regs: RegisterBatch = [[0; BATCH]; 4];
                            for (i, r) in regs[Reg::Z as usize].iter_mut().enumerate() {
                                *r = z_base + i as i64;
                            }
                            execute_batch(&mut regs, block, input);
                            for (i, &out) in regs[Reg::Z as usize].iter().enumerate() {
                                let z = z_base + i as i64;
                                if (z < Z_LIMIT) && valid_output.contains(out) {
                                    valid_z_input.insert(z);
                                }
                            }
                        }
                    }
                    valid_z_input
                })
            })
            .collect::<Vec<_>>()
            .into_iter()
            .map(|handle| handle.join().unwrap())
            .reduce(|mut acc, other| {
                acc.union_with(&other);
                acc
            })
            .expect("at least one chunk")
    })
}

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

    if valid_z_output_cache.is_none() {
        let mut valid_z_output: ZOutputCache = array::from_fn(|_| ZSet::new());

        // Find valid z outputs for each block
        valid_z_output[NUM_BLOCKS - 1].insert(0);
        for (block_id, block) in blocks.iter().enumerate().rev() {
            let valid_z_input = valid_z_inputs(block, &valid_z_output[block_id]);
            if (block_id > 0) {
                valid_z_output[block_id - 1] = valid_z_input;
            }
        }
        *valid_z_output_cache = Some(valid_z_output);
    }
    let valid_z_output = valid_z_output_cache.as_ref().unwrap();

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
                return if valid_z_output[block_id].contains(regs[Reg::Z as usize]) {
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
