use std::convert::TryFrom;
use std::ops::{Add, AddAssign, Div, DivAssign, Mul, MulAssign, Rem, RemAssign, Sub, SubAssign};

pub trait Numeric:
    Add
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

fn int_sqrt_u64(n: u64) -> u64 {
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
    result.sort();
    result
}
