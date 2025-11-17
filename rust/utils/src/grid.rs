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

    pub fn position(&self, linear_index: usize) -> (usize, usize) {
        return (
            linear_index % self.num_columns,
            linear_index / self.num_columns,
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
