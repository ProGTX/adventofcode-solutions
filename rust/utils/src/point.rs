use std::hash::{Hash, Hasher};
use std::ops::{Add, AddAssign, Div, DivAssign, Mul, MulAssign, Neg, Sub, SubAssign};

#[derive(Clone, Copy, Debug, Default, Eq, PartialEq, PartialOrd, Ord)]
pub struct Point<T> {
    pub x: T,
    pub y: T,
}

impl<T> Point<T> {
    pub const fn new(x: T, y: T) -> Self {
        Point { x, y }
    }

    // TODO: Change delimiter to Pattern trait once it's stable
    pub fn parse(s: &str, delimiter: &str) -> Option<Self>
    where
        T: std::str::FromStr,
    {
        let (x, y) = s.split_once(delimiter)?;
        Some(Self {
            x: x.parse::<T>().ok()?,
            y: y.parse::<T>().ok()?,
        })
    }

    pub fn cast<U>(&self) -> Point<U>
    where
        U: From<T>,
        T: Clone,
    {
        Point::<U> {
            x: U::from(self.x.clone()),
            y: U::from(self.y.clone()),
        }
    }

    pub fn min(self, other: Self) -> Self
    where
        T: Copy + Ord,
    {
        Self {
            x: self.x.min(other.x),
            y: self.y.min(other.y),
        }
    }

    pub fn max(self, other: Self) -> Self
    where
        T: Copy + Ord,
    {
        Self {
            x: self.x.max(other.x),
            y: self.y.max(other.y),
        }
    }

    pub fn abs(&self) -> Self
    where
        T: Neg<Output = T> + Copy + PartialOrd + Default,
    {
        Self {
            x: if (self.x >= T::default()) {
                self.x
            } else {
                -self.x
            },
            y: if (self.y >= T::default()) {
                self.y
            } else {
                -self.y
            },
        }
    }

    pub fn normal(&self) -> Self
    where
        T: Neg<Output = T> + Div<Output = T> + Copy + PartialOrd + Default,
    {
        *self / self.abs()
    }

    pub fn safe_divide(self, rhs: Self, default: T) -> Self
    where
        T: PartialEq + Default + Div<Output = T> + Copy,
    {
        Self {
            x: if (rhs.x == T::default()) {
                default
            } else {
                self.x / rhs.x
            },
            y: if (rhs.y == T::default()) {
                default
            } else {
                self.y / rhs.y
            },
        }
    }
}

// Tuple to Point
impl<T> From<(T, T)> for Point<T> {
    fn from((x, y): (T, T)) -> Self {
        Point { x, y }
    }
}
// Point to Tuple
impl<T> From<Point<T>> for (T, T) {
    fn from(point: Point<T>) -> Self {
        (point.x, point.y)
    }
}

impl<T> Add for Point<T>
where
    T: Add<Output = T>,
{
    type Output = Self;

    fn add(self, rhs: Self) -> Self {
        Self {
            x: self.x + rhs.x,
            y: self.y + rhs.y,
        }
    }
}
impl<T> AddAssign for Point<T>
where
    T: AddAssign,
{
    fn add_assign(&mut self, rhs: Self) {
        self.x += rhs.x;
        self.y += rhs.y;
    }
}

impl<T> Sub for Point<T>
where
    T: Sub<Output = T>,
{
    type Output = Self;
    fn sub(self, rhs: Self) -> Self {
        Self {
            x: self.x - rhs.x,
            y: self.y - rhs.y,
        }
    }
}
impl<T> SubAssign for Point<T>
where
    T: SubAssign,
{
    fn sub_assign(&mut self, rhs: Self) {
        self.x -= rhs.x;
        self.y -= rhs.y;
    }
}

impl<T> Mul for Point<T>
where
    T: Mul<Output = T>,
{
    type Output = Self;

    fn mul(self, rhs: Self) -> Self {
        Self {
            x: self.x * rhs.x,
            y: self.y * rhs.y,
        }
    }
}
impl<T> MulAssign for Point<T>
where
    T: MulAssign,
{
    fn mul_assign(&mut self, rhs: Self) {
        self.x *= rhs.x;
        self.y *= rhs.y;
    }
}

impl<T> Div for Point<T>
where
    T: Div<Output = T>,
{
    type Output = Self;

    fn div(self, rhs: Self) -> Self {
        Self {
            x: self.x / rhs.x,
            y: self.y / rhs.y,
        }
    }
}
impl<T> DivAssign for Point<T>
where
    T: DivAssign,
{
    fn div_assign(&mut self, rhs: Self) {
        self.x /= rhs.x;
        self.y /= rhs.y;
    }
}

impl<T> Hash for Point<T>
where
    T: Clone,
    usize: From<T>,
{
    fn hash<H: Hasher>(&self, state: &mut H) {
        state.write_usize(usize::from(self.x.clone()));
        state.write_usize(usize::from(self.y.clone()));
    }
}
