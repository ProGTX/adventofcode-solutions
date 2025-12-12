use crate::{iter::CollectArrayVec, point::Point};
use arrayvec::ArrayVec;

pub type Ipos = Point<isize>;
pub type Upos = Point<usize>;

#[derive(Clone)]
pub struct Grid<T> {
    pub data: Vec<T>,
    pub num_rows: usize,
    pub num_columns: usize,
}

pub const BASIC_NEIGHBOR_DIFFS: [Ipos; 4] = [
    Point::new(0, -1),
    Point::new(-1, 0),
    Point::new(1, 0),
    Point::new(0, 1),
];

pub const ALL_NEIGHBOR_DIFFS: [Ipos; 8] = [
    Point::new(-1, -1),
    Point::new(0, -1),
    Point::new(1, -1),
    Point::new(-1, 0),
    Point::new(1, 0),
    Point::new(-1, 1),
    Point::new(0, 1),
    Point::new(1, 1),
];

impl<T> Grid<T> {
    pub fn new(value: T, num_rows: usize, num_columns: usize) -> Self
    where
        T: Clone,
    {
        Grid {
            data: vec![value; num_rows * num_columns],
            num_rows,
            num_columns,
        }
    }

    pub fn from_vec(data: Vec<T>, num_rows: usize, num_columns: usize) -> Self {
        assert_eq!(
            data.len(),
            num_rows * num_columns,
            "data length {} doesn't match grid dimensions {}x{}",
            data.len(),
            num_rows,
            num_columns
        );
        Grid {
            data,
            num_rows,
            num_columns,
        }
    }

    pub fn from_iter(
        iter: impl IntoIterator<Item = T>,
        num_rows: usize,
        num_columns: usize,
    ) -> Self {
        Self::from_vec(iter.into_iter().collect(), num_rows, num_columns)
    }

    pub const fn linear_index(&self, row: usize, column: usize) -> usize {
        row * self.num_columns + column
    }

    pub const fn linear_from_pos(&self, pos: Upos) -> usize {
        self.linear_index(pos.y, pos.x)
    }

    /// Returns (column, row)
    pub const fn position(&self, linear_index: usize) -> Upos {
        return Upos::new(
            linear_index % self.num_columns, // column
            linear_index / self.num_columns, // row
        );
    }

    pub fn get(&self, row: usize, column: usize) -> &T {
        let index = self.linear_index(row, column);
        &self.data[index]
    }
    pub fn get_mut(&mut self, row: usize, column: usize) -> &mut T {
        let index = self.linear_index(row, column);
        &mut self.data[index]
    }

    pub fn modify(&mut self, value: T, row: usize, column: usize) {
        let index = self.linear_index(row, column);
        self.data[index] = value;
    }

    pub fn row(&self, row: usize) -> &[T] {
        let row_start = self.linear_index(row, 0);
        &self.data[row_start..row_start + self.num_columns]
    }
    pub fn row_mut(&mut self, row: usize) -> &mut [T] {
        let row_start = self.linear_index(row, 0);
        &mut self.data[row_start..row_start + self.num_columns]
    }
    pub fn for_row_mut<F>(&mut self, row: usize, mut f: F)
    where
        F: FnMut(&mut T),
    {
        for column in 0..self.num_rows {
            let index = self.linear_index(row, column);
            f(&mut self.data[index]);
        }
    }

    pub fn column(&self, column: usize) -> impl Iterator<Item = &T> {
        (0..self.num_rows).map(move |row| &self.data[self.linear_index(row, column)])
    }
    pub fn for_column_mut<F>(&mut self, column: usize, mut f: F)
    where
        F: FnMut(&mut T),
    {
        for row in 0..self.num_rows {
            let index = self.linear_index(row, column);
            f(&mut self.data[index]);
        }
    }

    pub fn in_bounds_unsigned(&self, row: usize, column: usize) -> bool {
        return (row < self.num_rows) && (column < self.num_columns);
    }

    pub fn in_bounds_signed(&self, row: isize, column: isize) -> bool {
        return (row >= 0)
            && (column >= 0)
            && self.in_bounds_unsigned(row as usize, column as usize);
    }

    fn get_neighbor_positions<const N: usize>(
        &self,
        neighbor_diffs: &[Ipos; N],
        ipos: Ipos,
    ) -> ArrayVec<Upos, N> {
        let mut neighbors = ArrayVec::new();
        for neighbor_diff in neighbor_diffs {
            let neighbor_pos = ipos + *neighbor_diff;
            if (self.in_bounds_signed(neighbor_pos.y, neighbor_pos.x)) {
                neighbors.push(Upos {
                    x: neighbor_pos.x as usize,
                    y: neighbor_pos.y as usize,
                });
            }
        }
        return neighbors;
    }

    pub fn basic_neighbor_positions(&self, upos: Upos) -> ArrayVec<Upos, 4> {
        self.get_neighbor_positions(
            &BASIC_NEIGHBOR_DIFFS,
            Ipos::new(upos.x as isize, upos.y as isize),
        )
    }

    pub fn all_neighbor_positions(&self, upos: Upos) -> ArrayVec<Upos, 8> {
        self.get_neighbor_positions(
            &ALL_NEIGHBOR_DIFFS,
            Ipos::new(upos.x as isize, upos.y as isize),
        )
    }

    fn get_neighbor_values<const N: usize>(
        &self,
        neighbor_diffs: &[Ipos; N],
        ipos: Ipos,
    ) -> ArrayVec<T, N>
    where
        T: Copy,
    {
        self.get_neighbor_positions(neighbor_diffs, ipos)
            .iter()
            .map(|neighbor_pos| self.get(neighbor_pos.y, neighbor_pos.x).clone())
            .collect_array_vec::<N>()
    }

    pub fn basic_neighbor_values(&self, upos: Upos) -> ArrayVec<T, 4>
    where
        T: Copy,
    {
        self.get_neighbor_values(
            &BASIC_NEIGHBOR_DIFFS,
            Ipos::new(upos.x as isize, upos.y as isize),
        )
    }

    pub fn all_neighbor_values(&self, upos: Upos) -> ArrayVec<T, 8>
    where
        T: Copy,
    {
        self.get_neighbor_values(
            &ALL_NEIGHBOR_DIFFS,
            Ipos::new(upos.x as isize, upos.y as isize),
        )
    }

    pub fn print(&self)
    where
        T: std::fmt::Display,
    {
        for row in self.data.chunks(self.num_columns) {
            println!(
                "{}",
                row.iter()
                    .map(|value| value.to_string())
                    .collect::<String>()
            );
        }
    }
}

#[derive(Clone, Debug, Default)]
pub struct ConfigInput {
    pub padding: Option<char>,
    pub start_char: Option<char>,
    pub end_char: Option<char>,
}

#[derive(Clone, Debug, Default)]
pub struct ConfigOutput {
    pub start_pos: Option<Upos>,
    pub end_pos: Option<Upos>,
}

impl Grid<char> {
    pub fn from_file_config(filename: &str, config: ConfigInput) -> (Self, ConfigOutput) {
        let lines = std::fs::read_to_string(filename)
            .unwrap()
            .lines()
            .map(|line| line.to_string())
            .collect::<Vec<String>>();
        let padding = config.padding.is_some();
        let mut output_config = ConfigOutput::default();
        let set_once = |optional_pos: &mut Option<Upos>,
                        optional_char: &Option<char>,
                        line: &String,
                        row_id| {
            *optional_pos = optional_pos.or_else(|| {
                optional_char.and_then(|input_char| {
                    line.chars()
                        .position(|c| c == input_char)
                        .map(|column_id| Upos {
                            x: column_id + (padding as usize),
                            y: row_id,
                        })
                })
            });
        };
        let mut grid = {
            let mut padded_data = Vec::<char>::new();
            let num_rows = lines.len() + 2 * (padding as usize);
            let num_columns = lines[0].len() + 2 * (padding as usize);
            if (padding) {
                padded_data.extend(std::iter::repeat(config.padding.unwrap()).take(num_columns));
            }
            let mut row_id = (padding as usize);
            for line in lines {
                if (padding) {
                    padded_data.push(config.padding.unwrap());
                }
                set_once(
                    &mut output_config.start_pos,
                    &config.start_char,
                    &line,
                    row_id,
                );
                set_once(&mut output_config.end_pos, &config.end_char, &line, row_id);
                padded_data.extend(line.chars());
                if (padding) {
                    padded_data.push(config.padding.unwrap());
                }
                row_id += 1;
            }
            if (padding) {
                padded_data.extend(std::iter::repeat(config.padding.unwrap()).take(num_columns));
            }
            Grid {
                data: padded_data,
                num_rows: num_rows,
                num_columns: num_columns,
            }
        };
        let size = grid.num_rows * grid.num_columns;
        // Ensure the data is a proper rectangle
        // TODO: What to do if we're cutting off data?
        // That's probably ill-formed data anyway.
        grid.data.resize(size, config.padding.unwrap_or(' '));
        (grid, output_config)
    }

    pub fn from_file(filename: &str) -> Self {
        Self::from_file_config(filename, ConfigInput::default()).0
    }
}

impl<T> Grid<T> {
    pub fn from_char_grid(char_grid: &Grid<char>) -> Self
    where
        T: From<u32>,
    {
        Grid {
            data: char_grid
                .data
                .iter()
                .map(|c| T::from(c.to_digit(10).unwrap()))
                .collect(),
            num_rows: char_grid.num_rows,
            num_columns: char_grid.num_columns,
        }
    }
}

pub fn from_file<T>(filename: &str) -> Grid<T>
where
    T: From<u32>,
{
    Grid::from_char_grid(&Grid::<char>::from_file(filename))
}
