use aoc::string::NameToId;

const OP_AND: usize = 0;
const OP_OR: usize = 1;
const OP_XOR: usize = 2;

// Length of a wire name like "x00"
const WIRE_PREFIX_LEN: usize = 3;
const UNINIT_WIRE: usize = usize::MAX;

#[derive(Clone, Copy)]
struct Gate {
    lhs: usize,
    rhs: usize,
    op: usize,
}
impl Gate {
    fn get(&self, wires: &[usize]) -> usize {
        debug_assert!(self.op != UNINIT_WIRE, "Invalid gate");
        match self.op {
            OP_AND => wires[self.lhs] & wires[self.rhs],
            OP_OR => wires[self.lhs] | wires[self.rhs],
            OP_XOR => wires[self.lhs] ^ wires[self.rhs],
            _ => unreachable!("Invalid op"),
        }
    }
}

struct Input {
    wires: Vec<usize>,
    outputs: Vec<Gate>,
    z_ids: Vec<usize>,
}

fn record_z_id(name: &str, id: usize, z_ids: &mut Vec<usize>) {
    if let Some(digits) = name.strip_prefix('z') {
        let digit: usize = digits.parse().unwrap();
        if z_ids.len() <= digit {
            z_ids.resize(digit + 1, UNINIT_WIRE);
        }
        z_ids[digit] = id;
    }
}

fn parse(filename: &str) -> Input {
    let mut name_to_id = NameToId::new();
    let mut wires = Vec::new();
    let mut outputs = Vec::new();
    let mut z_ids = Vec::new();
    let mut parsing_gates = false;

    for line in aoc::file::read_lines(filename) {
        if line.is_empty() {
            parsing_gates = true;
            continue;
        }
        if !parsing_gates {
            let name = &line[..WIRE_PREFIX_LEN];
            let id = name_to_id.intern(name);
            wires.resize(name_to_id.new_len(wires.len()), UNINIT_WIRE);
            let value: usize = line[WIRE_PREFIX_LEN + 2..].parse().unwrap();
            wires[id] = value;
            record_z_id(name, id, &mut z_ids);
        } else {
            let (input_str, output_str) = line.split_once('-').unwrap();
            let output_name = &output_str[2..];
            let output_id = name_to_id.intern(output_name);
            wires.resize(name_to_id.new_len(wires.len()), UNINIT_WIRE);
            record_z_id(output_name, output_id, &mut z_ids);

            let parts: Vec<&str> = input_str.split(' ').collect();
            let lhs_name = parts[0];
            let op_str = parts[1];
            let rhs_name = parts[2];
            let lhs_id = name_to_id.intern(lhs_name);
            let rhs_id = name_to_id.intern(rhs_name);
            wires.resize(name_to_id.new_len(wires.len()), UNINIT_WIRE);

            let op_id = match op_str {
                "AND" => OP_AND,
                "OR" => OP_OR,
                "XOR" => OP_XOR,
                _ => unreachable!("Invalid op"),
            };

            outputs.resize(
                name_to_id.new_len(outputs.len()),
                Gate {
                    lhs: 0,
                    rhs: 0,
                    op: UNINIT_WIRE,
                },
            );
            outputs[output_id] = Gate {
                lhs: lhs_id,
                rhs: rhs_id,
                op: op_id,
            };
        }
    }

    Input {
        wires,
        outputs,
        z_ids,
    }
}

fn solve_wire(wires: &mut [usize], outputs: &[Gate], wire_id: usize) {
    let value = wires[wire_id];
    if value != UNINIT_WIRE {
        return;
    }
    let output_gate = outputs[wire_id];
    solve_wire(wires, outputs, output_gate.lhs);
    solve_wire(wires, outputs, output_gate.rhs);
    wires[wire_id] = output_gate.get(wires);
}

fn solve_wires(mut wires: Vec<usize>, outputs: &[Gate]) -> Vec<usize> {
    for w in 0..wires.len() {
        solve_wire(&mut wires, outputs, w);
    }
    wires
}

fn get_z(wires: &[usize], z_ids: &[usize]) -> usize {
    let mut number = 0;
    let mut multiplier = 1;
    for &z_id in z_ids {
        number += multiplier * wires[z_id];
        multiplier *= 2;
    }
    number
}

fn solve_case1(input: &Input) -> usize {
    let wires = solve_wires(input.wires.clone(), &input.outputs);
    get_z(&wires, &input.z_ids)
}

fn main() {
    println!("Part 1");
    let example = parse("day24.example");
    aoc::expect_result!(4, solve_case1(&example));
    let example2 = parse("day24.example2");
    aoc::expect_result!(2024, solve_case1(&example2));
    let input = parse("day24.input");
    aoc::expect_result!(64755511006320, solve_case1(&input));

    println!("Part 2");
    aoc::return_incomplete();
    // aoc::expect_result!("aaa,aoc,bbb,ccc,eee,ooo,z24,z99", solve_case2(&example));
    // aoc::expect_result!("1337", solve_case2(&input));
}
