use std::collections::HashMap;

type Signal = bool;
type Inputs = HashMap<usize, Signal>;
type Outputs = Vec<usize>;

const BROADCASTER_STR: &str = "broadcaster";
const BUTTON_ID: usize = usize::MAX;

#[derive(Clone)]
enum Module {
    Output {
        outputs: Outputs,
    },
    FlipFlop {
        inputs: Inputs,
        outputs: Outputs,
        state: Signal,
    },
    Conjunction {
        inputs: Inputs,
        outputs: Outputs,
    },
    Broadcast {
        inputs: Inputs,
        outputs: Outputs,
    },
}
impl Default for Module {
    fn default() -> Self {
        Module::Output {
            outputs: Vec::new(),
        }
    }
}

struct Input {
    module_map: Vec<Module>,
    broadcaster_id: usize,
    rx_id: Option<usize>,
}

fn parse(filename: &str) -> Input {
    let lines = aoc::file::read_lines(filename);

    let mut name_to_id = aoc::string::NameToId::new();
    let broadcaster_id = name_to_id.intern(BROADCASTER_STR);

    let mut module_map = Vec::new();
    let mut input_map = HashMap::<usize, Vec<usize>>::new();

    for line in &lines {
        let (from, to) = line.split_once(" -> ").unwrap();
        let name = if from.starts_with('%') || from.starts_with('&') {
            &from[1..]
        } else {
            from
        };
        let id = name_to_id.intern(name);
        module_map.resize_with(name_to_id.new_len(module_map.len()), Module::default);

        let mut outputs: Outputs = Vec::new();
        for out_name in to.split(", ") {
            let out_id = name_to_id.intern(out_name);
            module_map.resize_with(name_to_id.new_len(module_map.len()), Module::default);
            outputs.push(out_id);
            input_map.entry(out_id).or_default().push(id);
        }

        if from.starts_with('%') {
            module_map[id] = Module::FlipFlop {
                inputs: Inputs::new(),
                outputs,
                state: false,
            };
        } else if from.starts_with('&') {
            module_map[id] = Module::Conjunction {
                inputs: Inputs::new(),
                outputs,
            };
        } else if from == BROADCASTER_STR {
            module_map[id] = Module::Broadcast {
                inputs: Inputs::new(),
                outputs,
            };
        }
    }

    // Only conjunctions with more than one input need to have inputs updated
    for (id, inputs) in &input_map {
        if inputs.len() < 2 {
            continue;
        }
        for &input_id in inputs {
            module_add_input(&mut module_map[*id], input_id, false);
        }
    }

    Input {
        module_map,
        broadcaster_id,
        rx_id: name_to_id.get("rx").copied(),
    }
}

fn module_outputs(module: &Module) -> &Outputs {
    match module {
        Module::Output { outputs }
        | Module::FlipFlop { outputs, .. }
        | Module::Conjunction { outputs, .. }
        | Module::Broadcast { outputs, .. } => outputs,
    }
}

fn module_add_input(module: &mut Module, name: usize, input: Signal) {
    match module {
        Module::Output { .. } => {
            // Output module ignores its inputs
        }
        Module::FlipFlop { inputs, .. } => {
            inputs.clear();
            inputs.insert(name, input);
        }
        Module::Conjunction { inputs, .. } | Module::Broadcast { inputs, .. } => {
            inputs.insert(name, input);
        }
    }
}

fn module_process(module: &mut Module) -> Option<Signal> {
    match module {
        Module::FlipFlop { inputs, state, .. } => {
            assert_eq!(inputs.len(), 1, "Flip flop requires exactly one input");
            if *inputs.values().next().unwrap() {
                // Ignore high pulse
                return None;
            }
            *state = !*state;
            Some(*state)
        }
        Module::Conjunction { inputs, .. } => Some(!inputs.values().all(|&v| v)),
        Module::Broadcast { inputs, .. } => {
            assert_eq!(inputs.len(), 1, "Broadcast requires exactly one input");
            Some(*inputs.values().next().unwrap())
        }
        Module::Output { .. } => None,
    }
}

#[derive(Clone, Copy)]
struct SignalTransit {
    from: usize,
    signal: Signal,
    to: usize,
}

#[derive(Default)]
struct PushResult {
    num_pulses: [i32; 2],
    target_received_low: bool,
}

fn push_button(
    module_map: &mut [Module],
    broadcaster_id: usize,
    target_id: Option<usize>,
) -> PushResult {
    let mut result = PushResult::default();
    let mut remaining_signals = vec![SignalTransit {
        from: BUTTON_ID,
        signal: false,
        to: broadcaster_id,
    }];
    result.num_pulses[0] += 1;

    let mut signals_start = 0;
    while signals_start < remaining_signals.len() {
        let current_signal = remaining_signals[signals_start];
        signals_start += 1;

        let current = &mut module_map[current_signal.to];
        module_add_input(current, current_signal.from, current_signal.signal);

        let Some(signal) = module_process(current) else {
            continue;
        };
        let outputs = module_outputs(current);
        for &out in outputs {
            remaining_signals.push(SignalTransit {
                from: current_signal.to,
                signal,
                to: out,
            });
            result.num_pulses[signal as usize] += 1;
            if target_id == Some(out) && !signal {
                result.target_received_low = true;
            }
        }
    }
    result
}

fn solve_case1(input: &Input) -> u64 {
    let mut module_map = input.module_map.clone();
    let mut total_signals = [0u64; 2];
    for _ in 0..1000 {
        let result = push_button(&mut module_map, input.broadcaster_id, None);
        total_signals[0] += result.num_pulses[0] as u64;
        total_signals[1] += result.num_pulses[1] as u64;
    }
    total_signals[0] * total_signals[1]
}

fn solve_case2(input: &Input) -> u64 {
    let mut module_map = input.module_map.clone();
    let rx_id = input.rx_id.expect("rx module not found");
    for num_presses in 1u64.. {
        let result = push_button(&mut module_map, input.broadcaster_id, Some(rx_id));
        if result.target_received_low {
            return num_presses;
        }
    }
    unreachable!()
}

fn main() {
    println!("Part 1");
    let example = parse("day20.example");
    aoc::expect_result!(32000000, solve_case1(&example));
    let example2 = parse("day20.example2");
    aoc::expect_result!(11687500, solve_case1(&example2));
    let input = parse("day20.input");
    aoc::expect_result!(814934624, solve_case1(&input));

    println!("Part 2");
    aoc::return_incomplete();
    // aoc::expect_result!(1337, solve_case2(&input));
}
