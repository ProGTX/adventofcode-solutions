use std::marker::PhantomData;

const WORD_BITS: usize = 64;

/// Integer types that can be stored in a [`BitmapSet`].
pub trait BitmapKey: Copy {
    /// Negative values wrap around to a huge index,
    /// so a single comparison against the limit
    /// covers both ends of the range.
    fn to_index(self) -> usize;
}

macro_rules! impl_bitmap_key {
    ($($type:ty),*) => {
        $(
            impl BitmapKey for $type {
                fn to_index(self) -> usize {
                    self as usize
                }
            }
        )*
    };
}
impl_bitmap_key!(i8, i16, i32, i64, isize, u8, u16, u32, u64, usize);

/// A set of integers stored as a bitmap:
/// one bit per value in [0, LIMIT), packed 64 to a word.
/// The value is its own index, so a lookup is a shift, a mask, and one load,
/// with no hashing and no probing.
///
/// A bitmap is worth it when the range of possible values
/// is small enough to keep in cache,
/// and when lookups far outnumber insertions.
/// It costs LIMIT/8 bytes no matter how few values it holds,
/// so it is a poor fit for a sparse set over a wide range.
///
/// Values outside [0, LIMIT) are never members:
/// [`BitmapSet::contains`] reports them as absent rather than panicking,
/// because producers routinely overshoot the range.
/// [`BitmapSet::insert`] does require its argument to be in range.
pub struct BitmapSet<K: BitmapKey, const LIMIT: usize> {
    bits: Vec<u64>,
    // Tell the compiler to pretend this struct contains a K.
    // K only appears in the method signatures, and a type parameter
    // that no field mentions is a compile error (E0392),
    // so this zero-sized marker is what ties K to the type
    key: PhantomData<K>,
}

impl<K: BitmapKey, const LIMIT: usize> BitmapSet<K, LIMIT> {
    /// An empty set covering the whole [0, LIMIT) range.
    pub fn new() -> Self {
        const { assert!(LIMIT > 0, "LIMIT must be positive") };
        BitmapSet {
            bits: vec![0; LIMIT.div_ceil(WORD_BITS)],
            key: PhantomData,
        }
    }

    /// Whether a value can be a member at all.
    pub fn in_range(value: K) -> bool {
        value.to_index() < LIMIT
    }

    /// Membership test for an arbitrary value.
    /// Out of range values are not members.
    pub fn contains(&self, value: K) -> bool {
        if !Self::in_range(value) {
            return false;
        }
        let index = value.to_index();
        (self.bits[index / WORD_BITS] >> (index % WORD_BITS)) & 1 != 0
    }

    /// Adds a value, which must be in range.
    /// Returns whether it was not already a member,
    /// so a lookup and an insertion collapse into a single operation.
    pub fn insert(&mut self, value: K) -> bool {
        debug_assert!(Self::in_range(value), "Value must be in [0, LIMIT)");
        let index = value.to_index();
        let mask = 1u64 << (index % WORD_BITS);
        let word = &mut self.bits[index / WORD_BITS];
        let was_member = (*word & mask) != 0;
        *word |= mask;
        !was_member
    }

    /// Removes all values, keeping the allocation.
    pub fn clear(&mut self) {
        self.bits.fill(0);
    }

    /// Folds another set into this one.
    pub fn union_with(&mut self, other: &Self) {
        for (bits, other_bits) in self.bits.iter_mut().zip(&other.bits) {
            *bits |= other_bits;
        }
    }
}

impl<K: BitmapKey, const LIMIT: usize> Default for BitmapSet<K, LIMIT> {
    fn default() -> Self {
        Self::new()
    }
}
