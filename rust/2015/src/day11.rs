fn to_numbers(s: &str) -> Vec<u32> {
    s.bytes().map(|c| (c - b'a') as u32).collect::<Vec<u32>>()
}
const I_NUMBER: u32 = b'i' as u32 - b'a' as u32;
const L_NUMBER: u32 = b'l' as u32 - b'a' as u32;
const O_NUMBER: u32 = b'o' as u32 - b'a' as u32;
const Y_NUMBER: u32 = b'y' as u32 - b'a' as u32;
const Z_NUMBER: u32 = b'z' as u32 - b'a' as u32;

fn is_invalid_number(n: u32) -> bool {
    matches!(n, I_NUMBER | L_NUMBER | O_NUMBER)
}

fn to_string(numbers: &[u32]) -> String {
    numbers
        .iter()
        .map(|&n| char::from_u32(n + (b'a' as u32)).unwrap())
        .collect()
}

// After calling this function we know we don't have any invalid letters
fn remove_initial_invalid(password: &mut [u32]) {
    let invalid_index = password.iter().position(|&n| is_invalid_number(n));
    if invalid_index.is_none() {
        // Nothing to do
        return;
    }
    // Increase the invalid letter and zero out the rest
    let invalid_index = invalid_index.unwrap();
    debug_assert!(!is_invalid_number(Z_NUMBER));
    password[invalid_index] += 1;
    debug_assert!(!is_invalid_number(password[invalid_index]));
    password[invalid_index + 1..].fill(0);
}

fn increasing_straight_index(password: &[u32]) -> (usize, usize) {
    if password.len() < 3 {
        return (0, 0);
    }
    let (mut index, mut size) = (0, 0);
    for i in 0..password.len() {
        let mut current_letter = password[i];
        if matches!(current_letter, Y_NUMBER | Z_NUMBER) {
            continue;
        }
        let mut current_size = 1;
        for j in i + 1..password.len() {
            current_letter += 1;
            if current_letter != password[j] {
                break;
            }
            current_size += 1;
            if current_letter == Z_NUMBER {
                break;
            }
        }
        if current_size >= 3 {
            index = i;
            size = current_size;
            break;
        }
    }
    (index, size)
}

fn has_increasing_straight(password: &[u32]) -> bool {
    let (index, size) = increasing_straight_index(password);
    if size < 3 {
        return false;
    }
    let (_, size) = increasing_straight_index(&password[index + size..]);
    if size > 0 {
        return false;
    }
    return true;
}

fn letter_pair_index(password: &[u32], start: usize) -> Option<usize> {
    debug_assert!(start < password.len() - 1);
    for index in start..password.len() - 1 {
        if password[index] == password[index + 1] {
            return Some(index);
        }
    }
    return None;
}

fn has_letter_pairs(password: &[u32]) -> bool {
    let first_index = letter_pair_index(password, 0);
    if first_index.is_none() {
        return false;
    }
    let mut start = first_index.unwrap();
    let first_pair_letter = password[start];
    // Search until we find a pair of different letters
    start += 2;
    while start < (password.len() - 1) {
        let second_index = letter_pair_index(password, start);
        if second_index.is_none() {
            return false;
        }
        start = second_index.unwrap();
        if password[start] != first_pair_letter {
            return true;
        }
        start += 2;
    }
    return false;
}

fn is_valid_password(password: &[u32]) -> bool {
    // 1. Exactly one increasing straight of at least three letters
    if !has_increasing_straight(password) {
        return false;
    }
    // 2. i, o, l are not allowed -> this is checked elsewhere
    // 3. At least two different non-overlapping pairs of letters
    return has_letter_pairs(password);
}

fn next_password(password: &Vec<u32>) -> Vec<u32> {
    let mut next = password.clone();
    let size = password.len();
    remove_initial_invalid(&mut next);
    while !is_valid_password(&next) {
        // Try increasing by one at the end of the password
        for index in (0..size).rev() {
            if next[index] != Z_NUMBER {
                // Increase the letter by at least one
                next[index] += 1 + is_invalid_number(next[index]) as u32;
                // Go check password validity
                break;
            }
            next[index] = 0;
            if index > 0 {
                // Go try increasing the letter before this one
                continue;
            }
            // Set to first valid password, aabcc
            debug_assert!(size >= 5);
            next[size - 1] = 2;
            next[size - 2] = 2;
            next[size - 3] = 1;
            debug_assert_eq!(next[size - 4], 0);
            debug_assert_eq!(next[size - 5], 0);
            break;
        }
    }
    next
}

fn solve_case(filename: &str) -> String {
    let password = to_numbers(std::fs::read_to_string(filename).unwrap().trim());
    let password = next_password(&password);
    to_string(&password)
}

fn main() {
    println!("Unit tests");
    let hijklmmn = to_numbers("hijklmmn");
    assert!(has_increasing_straight(&hijklmmn));
    let abbceffg = to_numbers("abbceffg");
    assert!(!has_increasing_straight(&abbceffg));
    assert!(has_letter_pairs(&abbceffg));
    assert!(!is_valid_password(&abbceffg));
    let abbcegjk = to_numbers("abbcegjk");
    assert!(!has_letter_pairs(&abbcegjk));
    assert!(!is_valid_password(&abbcegjk));
    let abcdffaa = to_numbers("abcdffaa");
    assert!(has_increasing_straight(&abcdffaa));
    assert!(has_letter_pairs(&abcdffaa));
    assert!(is_valid_password(&abcdffaa));
    let ghjaabcc = to_numbers("ghjaabcc");
    assert!(has_increasing_straight(&ghjaabcc));
    assert!(has_letter_pairs(&ghjaabcc));
    assert!(is_valid_password(&ghjaabcc));
    println!("Part 1");
    assert_eq!("ghjaabcc", solve_case("day11.example"));
    assert_eq!("hxbxxyzz", solve_case("day11.input"));
    // println!("Part 2");
    // assert_eq!(3369156, solve_case::<50>("day11.example"));
    // assert_eq!(5103798, solve_case::<50>("day11.input"));
}
