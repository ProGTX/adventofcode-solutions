type Pos = crate::point::Point<i32>;

#[derive(Clone, Copy, PartialEq, Eq, Debug, Hash, PartialOrd, Ord)]
pub enum Direction {
    East = 0,
    South = 1,
    West = 2,
    North = 3,
}

impl Direction {
    pub const fn diff(self) -> Pos {
        match self {
            Direction::East => Pos::new(1, 0),
            Direction::South => Pos::new(0, 1),
            Direction::West => Pos::new(-1, 0),
            Direction::North => Pos::new(0, -1),
        }
    }

    pub const fn clockwise(self) -> Self {
        match self {
            Direction::East => Direction::South,
            Direction::South => Direction::West,
            Direction::West => Direction::North,
            Direction::North => Direction::East,
        }
    }

    pub const fn counterclockwise(self) -> Self {
        match self {
            Direction::East => Direction::North,
            Direction::South => Direction::East,
            Direction::West => Direction::South,
            Direction::North => Direction::West,
        }
    }
}

pub const BASIC_DIRECTIONS: [Direction; 4] = [
    //
    Direction::East,
    Direction::West,
    Direction::South,
    Direction::North,
];

#[derive(Clone, Copy)]
pub struct Arrow {
    pub pos: Pos,
    pub dir: Direction,
}
