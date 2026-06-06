use aoc::grid::{ConfigInput, Grid};

type Algorithm = Vec<u32>;
type Image = aoc::grid::Grid<u32>;
type Input = (Algorithm, Image);

const PADDING: usize = 3;

fn parse(filename: &str) -> Input {
    let lines = aoc::file::read_lines(filename);
    let algorithm = lines[0].chars().map(|c| (c == '#') as u32).collect();
    const SKIP_LINES: usize = 2; // read_lines doesn't skip empty lines
    let char_image = Grid::from_lines_config(&lines[SKIP_LINES..], ConfigInput::default()).0;
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
    let background = (algorithm[0] + input_image.data[0]) % 2;
    let mut output_image = Image::new(
        background,
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
            output_image.modify(
                algorithm[alg_index],
                y as usize + PADDING,
                x as usize + PADDING,
            );
        }
    }
    return output_image;
}

fn solve_case<const ITERATIONS: usize>((algorithm, input_image): &Input) -> u32 {
    let mut image = input_image.clone();
    for _ in 0..ITERATIONS {
        image = apply(algorithm, &image);
    }
    image.data.iter().sum()
}

fn main() {
    println!("Part 1");

    let example = parse("day20.example");
    aoc::expect_result!(35, solve_case::<2>(&example));
    let input = parse("day20.input");
    aoc::expect_result!(5249, solve_case::<2>(&input));

    println!("Part 2");
    aoc::expect_result!(3351, solve_case::<50>(&example));
    aoc::expect_result!(15714, solve_case::<50>(&input));
}
