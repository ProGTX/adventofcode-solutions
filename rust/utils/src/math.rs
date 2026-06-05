use std::convert::TryFrom;
use std::ops::{Add, AddAssign, Div, DivAssign, Mul, MulAssign, Rem, RemAssign, Sub, SubAssign};

pub trait Numeric:
    Add<Output = Self>
    + Sub<Output = Self>
    + Mul<Output = Self>
    + Div<Output = Self>
    + Rem<Output = Self>
    + AddAssign
    + SubAssign
    + MulAssign
    + DivAssign
    + RemAssign
    + PartialOrd
    + Copy
    + Sized
    + From<u32>
    + From<i32>
{
}

impl<T> Numeric for T where
    T: Add<Output = Self>
        + Sub<Output = Self>
        + Mul<Output = Self>
        + Div<Output = Self>
        + Rem<Output = Self>
        + AddAssign
        + SubAssign
        + MulAssign
        + DivAssign
        + RemAssign
        + PartialOrd
        + Copy
        + Sized
        + From<u32>
        + From<i32>
{
}

pub fn gcd<T: Numeric>(a: T, b: T) -> T {
    let zero = T::from(0u32);
    if b == zero { a } else { gcd(b, a % b) }
}

pub fn prime_factors<T: Numeric>(n: T) -> Vec<T> {
    let mut result = Vec::new();
    let mut n = n;
    let mut z = T::from(2);
    while (z * z <= n) {
        if (n % z == T::from(0)) {
            result.push(z);
            n /= z;
        } else {
            z += T::from(1);
        }
    }
    if (n > T::from(1)) {
        result.push(n);
    }
    return result;
}

pub trait UnsignedOps:
    Copy
    + Ord
    + Into<u64>
    + TryFrom<u64>
    + std::ops::Rem<Output = Self>
    + std::ops::Div<Output = Self>
    + std::ops::Add<Output = Self>
{
}
impl UnsignedOps for u8 {}
impl UnsignedOps for u16 {}
impl UnsignedOps for u32 {}
impl UnsignedOps for u64 {}

pub fn int_sqrt_u64(n: u64) -> u64 {
    (n as f64).sqrt() as u64
}

pub fn divisors<T: UnsignedOps>(n: T) -> Vec<T> {
    let n_u64: u64 = n.into();
    let sqrt = int_sqrt_u64(n_u64);
    let mut result = Vec::new();
    let mut i_u64 = 1u64;
    while (i_u64 <= sqrt) {
        let i = T::try_from(i_u64).ok().unwrap();
        if ((n % i) == T::try_from(0u64).ok().unwrap()) {
            result.push(i);

            let other_u64 = n_u64 / i_u64;
            let other = T::try_from(other_u64).ok().unwrap();
            if (other != i) {
                result.push(other);
            }
        }
        i_u64 += 1;
    }
    result
}

pub fn binary_to_number<T>(bitset: &[u8]) -> T
where
    T: From<u8> + Copy + Add<Output = T> + Mul<Output = T>,
{
    bitset
        .iter()
        .rev()
        .fold((T::from(0_u8), T::from(1_u8)), |(acc, multiplier), &bit| {
            (
                acc + (T::from(bit) * multiplier),
                multiplier * T::from(2_u8),
            )
        })
        .0
}

#[derive(Clone, Copy, PartialEq, Eq)]
pub struct Ratio<T> {
    num: T,
    den: T,
}
impl<T: Numeric> Ratio<T> {
    pub fn new(num: T, den: T) -> Self {
        let zero = T::from(0u32);
        let one = T::from(1u32);
        if num == zero {
            return Self {
                num: zero,
                den: one,
            };
        }
        let abs_t = |n: T| -> T { if n < zero { zero - n } else { n } };
        let g = gcd(abs_t(num), abs_t(den));
        let sign = if den < zero { T::from(0u32) - one } else { one };
        Self {
            num: sign * (num / g),
            den: sign * (den / g),
        }
    }

    pub fn integer(n: T) -> Self {
        Self::new(n, T::from(1u32))
    }

    pub fn as_integer(self) -> T {
        self.num / self.den
    }
}

impl<T: Numeric> Add for Ratio<T> {
    type Output = Self;
    fn add(self, rhs: Self) -> Self {
        Self::new(self.num * rhs.den + rhs.num * self.den, self.den * rhs.den)
    }
}
impl<T: Numeric> Sub for Ratio<T> {
    type Output = Self;
    fn sub(self, rhs: Self) -> Self {
        Self::new(self.num * rhs.den - rhs.num * self.den, self.den * rhs.den)
    }
}
impl<T: Numeric> Mul for Ratio<T> {
    type Output = Self;
    fn mul(self, rhs: Self) -> Self {
        Self::new(self.num * rhs.num, self.den * rhs.den)
    }
}
impl<T: Numeric> Div for Ratio<T> {
    type Output = Self;
    fn div(self, rhs: Self) -> Self {
        Self::new(self.num * rhs.den, self.den * rhs.num)
    }
}
