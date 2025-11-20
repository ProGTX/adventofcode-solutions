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

// TODO: Make it generic
pub fn divisors(n: u32) -> Vec<u32> {
    let mut result = Vec::new();
    let sqrt_n = (n as f64).sqrt() as u32;
    for i in 1..=sqrt_n {
        if (n % i == 0) {
            result.push(i);
            if (i != (n / i)) {
                result.push(n / i);
            }
        }
    }
    result.sort();
    result
}
