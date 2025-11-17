pub struct Grid {
    pub data: Vec<u32>,
    pub num_rows: usize,
    pub num_columns: usize,
}

impl Grid {
    pub fn new(value: u32, num_rows: usize, num_columns: usize) -> Self {
        Grid {
            data: vec![value; num_rows * num_columns],
            num_rows,
            num_columns,
        }
    }
    pub fn linear_index(&self, row: usize, column: usize) -> usize {
        row * self.num_columns + column
    }
    pub fn at(&self, row: usize, column: usize) -> u32 {
        let index = self.linear_index(row, column);
        self.data[index]
    }
    pub fn modify(&mut self, value: u32, row: usize, column: usize) {
        let index = self.linear_index(row, column);
        self.data[index] = value;
    }
}
