use arrayvec::ArrayVec;

pub trait CollectArrayVec: Iterator {
    fn collect_array_vec<const N: usize>(self) -> ArrayVec<Self::Item, N>
    where
        Self: Sized,
    {
        let mut arr = ArrayVec::new();
        for item in self {
            arr.push(item);
        }
        arr
    }
}

// Implement for all iterators
impl<I: Iterator> CollectArrayVec for I {}
