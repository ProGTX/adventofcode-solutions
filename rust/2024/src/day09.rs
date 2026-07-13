const EMPTY_SPACE: i32 = -1;

fn parse(filename: &str) -> String {
    aoc::file::read_lines(filename).into_iter().next().unwrap()
}

fn expand(disk_map: &str) -> Vec<i32> {
    let mut filesystem = Vec::new();
    let mut id = 0;
    let mut is_empty = false;
    for current_char in disk_map.chars() {
        let num = current_char.to_digit(10).unwrap() as usize;
        let current = if !is_empty {
            let value = id;
            id += 1;
            value
        } else {
            EMPTY_SPACE
        };
        filesystem.extend(std::iter::repeat(current).take(num));
        is_empty = !is_empty;
    }
    filesystem
}

// Two pointers: `left` advances to the next empty cell from the front,
// `right` retreats to the next non-empty cell from the back,
// swap while they haven't crossed
fn compact(mut filesystem: Vec<i32>) -> Vec<i32> {
    let mut left = 0usize;
    let mut right = filesystem.len();
    loop {
        while right > 0 && filesystem[right - 1] == EMPTY_SPACE {
            right -= 1;
        }
        if right == 0 {
            break;
        }
        right -= 1;
        while left < filesystem.len() && filesystem[left] != EMPTY_SPACE {
            left += 1;
        }
        if left >= right {
            break;
        }
        filesystem.swap(left, right);
        left += 1;
    }
    filesystem
}

fn compact_nonfragment(mut filesystem: Vec<i32>) -> Vec<i32> {
    let mut right = filesystem.len();
    loop {
        // Skip over empty spaces from the back
        while right > 0 && filesystem[right - 1] == EMPTY_SPACE {
            right -= 1;
        }
        if right == 0 {
            break;
        }
        // Collect current elements into a range
        let current = filesystem[right - 1];
        let current_range_end = right;
        let mut current_range_begin = current_range_end;
        while current_range_begin > 0 && filesystem[current_range_begin - 1] == current {
            current_range_begin -= 1;
        }
        let current_size = current_range_end - current_range_begin;

        // Progress past the current range for the next iteration
        right = current_range_begin;

        // Find an empty range that can fit the current range
        if let Some(pos) = filesystem[..current_range_begin]
            .windows(current_size)
            .position(|window| window.iter().all(|&value| value == EMPTY_SPACE))
        {
            // Swap the current range with the empty space
            for offset in 0..current_size {
                filesystem.swap(pos + offset, current_range_begin + offset);
            }
        }
    }
    filesystem
}

fn checksum(filesystem: &[i32]) -> u64 {
    filesystem
        .iter()
        .enumerate()
        .map(|(position, &id)| {
            if id == EMPTY_SPACE {
                0
            } else {
                position as u64 * id as u64
            }
        })
        .sum()
}

fn solve_case1(disk_map: &str) -> u64 {
    checksum(&compact(expand(disk_map)))
}

fn solve_case2(disk_map: &str) -> u64 {
    checksum(&compact_nonfragment(expand(disk_map)))
}

fn main() {
    println!("Asserts");
    // "0..111....22222"
    assert_eq!(
        vec![0, -1, -1, 1, 1, 1, -1, -1, -1, -1, 2, 2, 2, 2, 2],
        expand("12345")
    );
    // "022111222......"
    assert_eq!(60, checksum(&[0, 2, 2, 1, 1, 1, 2, 2, 2]));
    // "0099811188827773336446555566.............."
    assert_eq!(
        1928,
        checksum(&[
            0, 0, 9, 9, 8, 1, 1, 1, 8, 8, 8, 2, 7, 7, 7, 3, 3, 3, 6, 4, 4, 6, 5, 5, 5, 5, 6, 6
        ])
    );
    assert_eq!(
        1928,
        checksum(&[
            0, 0, 9, 9, 8, 1, 1, 1, 8, 8, 8, 2, 7, 7, 7, 3, 3, 3, 6, 4, 4, 6, 5, 5, 5, 5, 6, 6, -1
        ])
    );
    assert_eq!(60, solve_case1("12345"));

    println!("Part 1");
    let example = parse("day09.example");
    aoc::expect_result!(1928, solve_case1(&example));
    let input = parse("day09.input");
    aoc::expect_result!(6432869891895u64, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(2858, solve_case2(&example));
    aoc::expect_result!(6467290479134u64, solve_case2(&input));
}
