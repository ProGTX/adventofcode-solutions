#[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub struct ClosedRange<T> {
    pub begin: T,
    pub end: T,
}

impl<T: Ord + Copy> ClosedRange<T> {
    pub fn new(begin: T, end: T) -> Self {
        if end < begin {
            ClosedRange {
                begin: end,
                end: begin,
            }
        } else {
            ClosedRange { begin, end }
        }
    }

    pub fn contains(self, other: Self) -> bool {
        other.begin >= self.begin && other.end <= self.end
    }

    pub fn overlaps_with(self, other: Self) -> bool {
        self.begin <= other.end && self.end >= other.begin
    }
}

impl<T: std::ops::Sub<Output = T> + Copy> ClosedRange<T> {
    pub fn direction(self) -> T {
        self.end - self.begin
    }

    pub fn size(self) -> T {
        self.direction()
    }
}
