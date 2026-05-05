#[derive(Clone, Copy, Debug)]
enum Reg {
    X = 0,
    Y = 1,
    Z = 2,
    W = 3,
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

fn parse_operand(s: &str) -> Operand {
    match s {
        "w" | "x" | "y" | "z" => Operand::Reg(parse_reg(s)),
        _ => Operand::Num(s.parse().expect("expected number")),
    }
}

fn parse(filename: &str) -> Vec<Instr> {
    aoc::file::read_lines(filename)
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
                parse_operand(parts[2])
            };
            Instr { op, dst, src }
        })
        .collect()
}

fn execute(mut regs: [i64; 4], instructions: &[Instr], input: &[i64]) -> [i64; 4] {
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

fn solve_case1(instructions: &[Instr]) -> u64 {
    // TODO: Implement Part 1
    0
}

fn solve_case2(instructions: &[Instr]) -> u64 {
    // TODO: Implement Part 2
    0
}

fn main() {
    println!("Part 1");

    let example = parse("day24.example");
    assert_eq!(0, solve_case1(&example));
    let input = parse("day24.input");
    // assert_eq!(XXX, solve_case1(&input));

    println!("Part 2");
    // assert_eq!(XXX, solve_case2(&example));
    // assert_eq!(XXX, solve_case2(&input));
}
