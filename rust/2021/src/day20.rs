use aoc::grid::{ConfigInput, Grid};

type Algorithm = Vec<u32>;
type Image = aoc::grid::Grid<u32>;
type Input = (Algorithm, Image);

const PADDING: usize = 3;

fn parse(filename: &str) -> Input {
    let lines = aoc::file::read_lines(filename);
    let algorithm = lines[0].chars().map(|c| (c == '#') as u32).collect();
    let char_image = Grid::from_lines_config(&lines[2..], ConfigInput::default()).0;
    // Add padding on each side to simulate the infinite image
    let mut image = Image::new(
        0,
        char_image.num_rows + 2 * PADDING,
        char_image.num_columns + 2 * PADDING,
    );
    for row in 0..char_image.num_rows {
        for col in 0..char_image.num_columns {
            *image.get_mut(row + PADDING, col + PADDING) =
                (*char_image.get(row, col) == '#') as u32;
        }
    }
    return (algorithm, image);
}

fn apply(algorithm: &Algorithm, input_image: &Image) -> Image {
    // The output image increases on each side
    let mut output_image = Image::new(
        (algorithm[0] + input_image.data[0]) % 2,
        input_image.num_rows + 2 * PADDING,
        input_image.num_columns + 2 * PADDING,
    );
    for y in 1..(input_image.num_rows - 1) as isize {
        for x in 1..(input_image.num_columns - 1) as isize {
            let mut bits: [u8; 9] = [0; 9];
            for dy in -1..=1 as isize {
                for dx in -1..=1 as isize {
                    let bit_index = (dy + 1) * 3 + dx + 1;
                    let pixel = input_image.get((y + dy) as usize, (x + dx) as usize);
                    bits[bit_index as usize] = *pixel as u8;
                }
            }
            let alg_index = aoc::math::binary_to_number::<usize>(&bits);
            output_image.modify(algorithm[alg_index], y as usize + PADDING, x as usize + PADDING);
        }
    }
    Grid::from_iter(
        output_image
            .data
            .iter()
            .map(|v| if (*v > 0) { '#' } else { '.' }),
        output_image.num_rows,
        output_image.num_columns,
    )
    .print();
    println!();
    return output_image;
}

fn solve_case1((algorithm, input_image): &Input) -> u32 {
    apply(algorithm, &apply(algorithm, input_image))
        .data
        .iter()
        .sum()
}

fn solve_case2((algorithm, input_image): &Input) -> u32 {
    // TODO: Implement Part 2
    0
}

fn main() {
    println!("Part 1");

    let example = parse("day20.example");
    assert_eq!(35, solve_case1(&example));
    let input = parse("day20.input");
    assert_eq!(5249, solve_case1(&input));

    println!("Part 2");
    // assert_eq!(XXX, solve_case2(&example));
    // assert_eq!(XXX, solve_case2(&input));
}
