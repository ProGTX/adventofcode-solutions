use arrayvec::ArrayVec;

#[derive(Clone)]
pub struct Grid<T> {
    pub data: Vec<T>,
    pub num_rows: usize,
    pub num_columns: usize,
}

pub const ALL_NEIGHBOR_DIFFS: [(i32, i32); 8] = [
    (-1, -1),
    (0, -1),
    (1, -1),
    (-1, 0),
    (1, 0),
    (-1, 1),
    (0, 1),
    (1, 1),
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

    pub fn linear_index(&self, row: usize, column: usize) -> usize {
        row * self.num_columns + column
    }

    /// Returns (column, row)
    pub fn position(&self, linear_index: usize) -> (usize, usize) {
        return (
            linear_index % self.num_columns, // column
            linear_index / self.num_columns, // row
        );
    }

    pub fn at(&self, row: usize, column: usize) -> T
    where
        T: Copy,
    {
        let index = self.linear_index(row, column);
        self.data[index]
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

    pub fn in_bounds_signed(&self, row: i32, column: i32) -> bool {
        return (row >= 0)
            && (column >= 0)
            && self.in_bounds_unsigned(row as usize, column as usize);
    }

    pub fn all_neighbor_values(&self, row: usize, column: usize) -> ArrayVec<T, 8>
    where
        T: Copy,
    {
        let mut neighbors = ArrayVec::<T, 8>::new();
        for neighbor_diff in ALL_NEIGHBOR_DIFFS {
            let neighbor_pos = (
                (column as i32) + neighbor_diff.0,
                (row as i32) + neighbor_diff.1,
            );
            if (self.in_bounds_signed(neighbor_pos.1, neighbor_pos.0)) {
                neighbors.push(self.at(neighbor_pos.1 as usize, neighbor_pos.0 as usize));
            }
        }
        return neighbors;
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

impl Grid<char> {
    pub fn from_file(filename: &str) -> Self {
        let lines = std::fs::read_to_string(filename)
            .unwrap()
            .trim()
            .lines()
            .map(|line| line.to_string())
            .collect::<Vec<String>>();
        Grid {
            data: lines.join("").chars().collect::<Vec<char>>(),
            num_rows: lines.len(),
            num_columns: lines[0].len(),
        }
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
