use aoc::string::NameToId;

// A signal can be a wire ID or an an actual value
// We unify them by storing values as 16-bit numbers,
// and the wire ID as numbers that are higher than all signal values
type SignalT = u32;
type WireIdT = SignalT;
type StrictSignalT = u16;
const FIRST_WIRE_ID: WireIdT = 1 << 16;
const fn is_value(signal: SignalT) -> bool {
    return signal < FIRST_WIRE_ID;
}
const fn input_id(wire_id: WireIdT) -> usize {
    assert!(wire_id >= FIRST_WIRE_ID, "Invalid wire ID");
    return (wire_id - FIRST_WIRE_ID) as usize;
}
const fn strict(signal: SignalT) -> StrictSignalT {
    assert!(is_value(signal), "Cannot convert provided signal to value");
    return signal as StrictSignalT;
}

#[derive(Clone)]
enum OperationT {
    ValueOp,
    AliasOp,
    NotOp,
    AndOp,
    OrOp,
    LShiftOp,
    RShiftOp,
}

#[derive(Clone)]
struct GateT {
    op: OperationT,
    lhs: SignalT,
    rhs: SignalT,
}

impl GateT {
    fn empty() -> GateT {
        GateT {
            op: OperationT::ValueOp,
            lhs: 0,
            rhs: 0,
        }
    }
}

// Include spaces to avoid trimming later
const ARROW_STR: &str = " -> ";
const NOT_STR: &str = "NOT ";
// Don't include spaces here because we've already trimmed spaces
const AND_STR: &str = "AND";
const OR_STR: &str = "OR";
const LSHIFT_STR: &str = "LSHIFT";
const RSHIFT_STR: &str = "RSHIFT";

fn parse(filename: &str) -> (Vec<GateT>, SignalT, SignalT) {
    let mut name_to_id = NameToId::from(FIRST_WIRE_ID as usize);
    let mut get_signal = |wire_name: &str| -> SignalT {
        if wire_name.chars().next().unwrap().is_digit(10) {
            return wire_name.parse().unwrap();
        }
        return name_to_id.intern(wire_name) as u32;
    };
    let mut inputs = Vec::<GateT>::new();
    for line in std::fs::read_to_string(filename).unwrap().lines() {
        let (gate_str, wire_name) = line.split_once(ARROW_STR).unwrap();
        let output_wire = get_signal(wire_name);
        let mut gate = GateT::empty();
        if gate_str.starts_with(NOT_STR) {
            gate.op = OperationT::NotOp;
            gate.rhs = get_signal(&gate_str[NOT_STR.len()..]);
        } else if !gate_str.contains(' ') {
            if gate_str.chars().next().unwrap().is_digit(10) {
                gate.op = OperationT::ValueOp;
            } else {
                gate.op = OperationT::AliasOp;
            }
            gate.lhs = get_signal(gate_str);
        } else {
            let parts = gate_str.split(' ').collect::<Vec<_>>();
            let [lhs, op, rhs] = parts.as_slice() else {
                panic!("Bad parsing");
            };
            gate.lhs = get_signal(lhs);
            gate.op = if *op == AND_STR {
                OperationT::AndOp
            } else if *op == OR_STR {
                OperationT::OrOp
            } else if *op == LSHIFT_STR {
                OperationT::LShiftOp
            } else if *op == RSHIFT_STR {
                OperationT::RShiftOp
            } else {
                unreachable!("Invalid operation!");
            };
            gate.rhs = get_signal(rhs);
        }

        let index = input_id(output_wire);
        inputs.resize(
            std::cmp::max(index + 1 as usize, inputs.len()),
            GateT::empty(),
        );
        inputs[index] = gate;
    }
    (inputs, get_signal("a"), get_signal("b"))
}

fn solve_wire(inputs: &mut [GateT], start: SignalT) -> StrictSignalT {
    if is_value(start) {
        return strict(start);
    }
    let index = input_id(start);
    let gate = inputs[index].clone();
    use OperationT::*;
    match gate.op {
        ValueOp => {
            return strict(gate.lhs);
        }
        AliasOp => {
            inputs[index].lhs = solve_wire(inputs, gate.lhs) as SignalT;
            return strict(inputs[index].lhs);
        }
        NotOp => {
            inputs[index].rhs = solve_wire(inputs, gate.rhs) as SignalT;
            return !strict(inputs[index].rhs);
        }
        _ => {
            inputs[index].lhs = solve_wire(inputs, gate.lhs) as SignalT;
            inputs[index].rhs = solve_wire(inputs, gate.rhs) as SignalT;
        }
    };
    let gate = &inputs[index];
    match gate.op {
        AndOp => {
            return strict(gate.lhs) & strict(gate.rhs);
        }
        OrOp => {
            return strict(gate.lhs) | strict(gate.rhs);
        }
        LShiftOp => {
            return strict(gate.lhs) << strict(gate.rhs);
        }
        RShiftOp => {
            return strict(gate.lhs) >> strict(gate.rhs);
        }
        _ => {
            unreachable!("Operation should have been covered before!");
        }
    }
}

fn solve_case1(filename: &str) -> StrictSignalT {
    let (mut inputs, a, _) = parse(filename);
    solve_wire(&mut inputs, a)
}

fn solve_case2(filename: &str) -> StrictSignalT {
    let (mut inputs, a, b) = parse(filename);
    let mut inputs_copy = inputs.clone();
    let result = solve_wire(&mut inputs_copy, a);
    inputs[input_id(b)] = GateT {
        op: OperationT::ValueOp,
        lhs: result as SignalT,
        rhs: 0,
    };
    solve_wire(&mut inputs, a)
}

fn main() {
    println!("Part 1");
    assert_eq!(114, solve_case1("day07.example"));
    assert_eq!(16076, solve_case1("day07.input"));
    println!("Part 2");
    assert_eq!(28, solve_case2("day07.example"));
    assert_eq!(2797, solve_case2("day07.input"));
}
