type Pos = crate::point::Point<i32>;

#[derive(Clone, Copy, PartialEq, Eq, Debug, Hash, PartialOrd, Ord)]
pub enum Direction {
    East = 0,
    South = 1,
    West = 2,
    North = 3,
    Southeast = 4,
    Southwest = 5,
    Northwest = 6,
    Northeast = 7,
}

impl Direction {
    pub const fn diff(self) -> Pos {
        match self {
            Direction::East => Pos::new(1, 0),
            Direction::South => Pos::new(0, 1),
            Direction::West => Pos::new(-1, 0),
            Direction::North => Pos::new(0, -1),
            Direction::Southeast => Pos::new(1, 1),
            Direction::Southwest => Pos::new(-1, 1),
            Direction::Northwest => Pos::new(-1, -1),
            Direction::Northeast => Pos::new(1, -1),
        }
    }

    pub const fn clockwise(self) -> Self {
        match self {
            Direction::East => Direction::South,
            Direction::South => Direction::West,
            Direction::West => Direction::North,
            Direction::North => Direction::East,
            _ => panic!("clockwise only valid for basic directions"),
        }
    }

    pub const fn counterclockwise(self) -> Self {
        match self {
            Direction::East => Direction::North,
            Direction::South => Direction::East,
            Direction::West => Direction::South,
            Direction::North => Direction::West,
            _ => panic!("counterclockwise only valid for basic directions"),
        }
    }
}

pub const BASIC_DIRECTIONS: [Direction; 4] = [
    Direction::East,
    Direction::West,
    Direction::South,
    Direction::North,
];

pub const ALL_SKY_DIRECTIONS: [Direction; 8] = [
    Direction::East,
    Direction::South,
    Direction::West,
    Direction::North,
    Direction::Southeast,
    Direction::Southwest,
    Direction::Northwest,
    Direction::Northeast,
];

#[derive(Clone, Copy, Debug, Hash, Eq, Ord, PartialEq, PartialOrd)]
pub struct Arrow {
    pub pos: Pos,
    pub dir: Direction,
}
