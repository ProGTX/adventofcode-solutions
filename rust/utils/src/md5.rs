//! MD5, calling the same implementation the C++ solutions use.
//! See `common/md5.h` for the code and the licence it came under

pub type Digest = [u8; 16];

/// How many messages one `aoc_md5_many` call takes at most
const MAX_BATCH: usize = 64;

/// The block a message is padded into, which `md5_fixed` works on
pub const BLOCK_SIZE: usize = 64;

/// How many messages a batch hashes side by side.
/// Eight 32-bit lanes is one AVX2 register and SSE2 splits it into two.
pub const LANES: usize = 8;

unsafe extern "C" {
    fn aoc_md5(message: *const u8, size: usize, digest: *mut Digest);
    fn aoc_md5_fixed(
        blocks: *mut [u8; BLOCK_SIZE],
        size: usize,
        count: usize,
        digests: *mut Digest,
    );
    fn aoc_md5_many(
        messages: *const *const u8,
        sizes: *const usize,
        count: usize,
        digests: *mut Digest,
    );
}

pub fn md5(input: &[u8]) -> Digest {
    let mut digest = Digest::default();
    unsafe { aoc_md5(input.as_ptr(), input.len(), &mut digest) };
    return digest;
}

/// Hashes one block per lane, in place:
/// every block holds a message of `size` bytes and zeroes after it.
/// Nothing is copied, so this is the one for a hot loop
pub fn md5_fixed(blocks: &mut [[u8; BLOCK_SIZE]], size: usize, digests: &mut [Digest]) {
    debug_assert!(digests.len() >= blocks.len());
    unsafe {
        aoc_md5_fixed(
            blocks.as_mut_ptr(),
            size,
            blocks.len(),
            digests.as_mut_ptr(),
        );
    }
}

/// Hashes messages that do not depend on each other, all at once:
/// same-size single-block ones go into parallel SIMD lanes
pub fn md5_many(inputs: &[&[u8]], digests: &mut [Digest]) {
    debug_assert!(digests.len() >= inputs.len());
    let mut pointers = [std::ptr::null(); MAX_BATCH];
    let mut sizes = [0; MAX_BATCH];
    for (first, batch) in inputs.chunks(MAX_BATCH).enumerate() {
        for (index, input) in batch.iter().enumerate() {
            pointers[index] = input.as_ptr();
            sizes[index] = input.len();
        }
        unsafe {
            aoc_md5_many(
                pointers.as_ptr(),
                sizes.as_ptr(),
                batch.len(),
                digests[first * MAX_BATCH..].as_mut_ptr(),
            );
        }
    }
}

pub const fn digest_to_hex(digest: Digest) -> [char; 32] {
    const HEX_CHARS: &Digest = b"0123456789abcdef";
    let mut s = ['0'; 32];
    // A `while` rather than a `for`, so this stays usable in a const context
    let mut i = 0;
    while i < digest.len() {
        let byte = digest[i];
        s[2 * i] = HEX_CHARS[(byte >> 4) as usize] as char;
        s[2 * i + 1] = HEX_CHARS[(byte & 0x0f) as usize] as char;
        i += 1;
    }
    s
}
