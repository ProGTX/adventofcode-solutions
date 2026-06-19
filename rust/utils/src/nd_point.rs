use std::ops::{Add, AddAssign, Div, DivAssign, Mul, MulAssign, Neg, Sub, SubAssign};

#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct NDPoint<T, const N: usize> {
    pub data: [T; N],
}

impl<T: Default, const N: usize> Default for NDPoint<T, N> {
    fn default() -> Self {
        NDPoint {
            data: std::array::from_fn(|_| T::default()),
        }
    }
}

impl<T, const N: usize> NDPoint<T, N> {
    pub fn from_fn(f: impl FnMut(usize) -> T) -> Self {
        NDPoint {
            data: std::array::from_fn(f),
        }
    }

    pub fn x(&self) -> &T {
        &self.data[0]
    }
    pub fn y(&self) -> &T {
        &self.data[1]
    }
    pub fn z(&self) -> &T {
        &self.data[2]
    }
    pub fn w(&self) -> &T {
        &self.data[3]
    }

    pub fn scale(self, factor: T) -> Self
    where
        T: Mul<Output = T> + Copy,
    {
        NDPoint::from_fn(|i| self.data[i] * factor)
    }

    pub fn x_mut(&mut self) -> &mut T {
        &mut self.data[0]
    }
    pub fn y_mut(&mut self) -> &mut T {
        &mut self.data[1]
    }
    pub fn z_mut(&mut self) -> &mut T {
        &mut self.data[2]
    }
    pub fn w_mut(&mut self) -> &mut T {
        &mut self.data[3]
    }
}

macro_rules! impl_pointwise_op {
    ($trait:ident, $method:ident, $assign_trait:ident, $assign_method:ident, $op:tt, $op_eq:tt) => {
        impl<T: $trait<Output = T> + Copy, const N: usize> $trait for NDPoint<T, N> {
            type Output = Self;
            fn $method(mut self, rhs: Self) -> Self {
                for i in 0..N { self.data[i] = self.data[i] $op rhs.data[i]; }
                self
            }
        }
        impl<T: $assign_trait + Copy, const N: usize> $assign_trait for NDPoint<T, N> {
            fn $assign_method(&mut self, rhs: Self) {
                for i in 0..N { self.data[i] $op_eq rhs.data[i]; }
            }
        }
    };
}

impl_pointwise_op!(Add, add, AddAssign, add_assign, +, +=);
impl_pointwise_op!(Sub, sub, SubAssign, sub_assign, -, -=);
impl_pointwise_op!(Mul, mul, MulAssign, mul_assign, *, *=);
impl_pointwise_op!(Div, div, DivAssign, div_assign, /, /=);

impl<T: Neg<Output = T> + Copy, const N: usize> Neg for NDPoint<T, N> {
    type Output = Self;
    fn neg(mut self) -> Self {
        for i in 0..N {
            self.data[i] = -self.data[i];
        }
        self
    }
}

impl<T: Neg<Output = T> + PartialOrd + Default + Copy, const N: usize> NDPoint<T, N> {
    pub fn abs(self) -> Self {
        NDPoint::from_fn(|i| {
            if self.data[i] < T::default() {
                -self.data[i]
            } else {
                self.data[i]
            }
        })
    }
}

impl<T: Add<Output = T> + Mul<Output = T> + Default + Copy, const N: usize> NDPoint<T, N> {
    pub fn dot(self, other: Self) -> T {
        (0..N).fold(T::default(), |acc, i| acc + self.data[i] * other.data[i])
    }

    pub fn sqr_magnitude(self) -> T {
        self.dot(self)
    }
}

pub fn distance_manhattan<T, const N: usize>(a: NDPoint<T, N>, b: NDPoint<T, N>) -> T
where
    T: Add<Output = T> + Sub<Output = T> + Neg<Output = T> + PartialOrd + Default + Copy,
{
    (b - a)
        .abs()
        .data
        .into_iter()
        .fold(T::default(), |acc, x| acc + x)
}

impl<T, const N: usize> IntoIterator for NDPoint<T, N> {
    type Item = T;
    type IntoIter = std::array::IntoIter<T, N>;
    fn into_iter(self) -> Self::IntoIter {
        self.data.into_iter()
    }
}

impl<'a, T, const N: usize> IntoIterator for &'a NDPoint<T, N> {
    type Item = &'a T;
    type IntoIter = std::slice::Iter<'a, T>;
    fn into_iter(self) -> Self::IntoIter {
        self.data.iter()
    }
}

impl<T: std::fmt::Display, const N: usize> std::fmt::Display for NDPoint<T, N> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "(")?;
        for (i, v) in self.data.iter().enumerate() {
            if i > 0 {
                write!(f, ", ")?;
            }
            write!(f, "{v}")?;
        }
        write!(f, ")")
    }
}
