pub type Digest = [u8; 16];

#[cfg(have_md5)]
unsafe extern "C" {
    /// OpenSSL's one-shot MD5, which is all these solutions ever need
    fn MD5(data: *const u8, size: usize, digest: *mut u8) -> *mut u8;
}

#[cfg(have_md5)]
pub fn md5(input: &[u8]) -> Digest {
    let mut digest = [0; 16];
    unsafe { MD5(input.as_ptr(), input.len(), digest.as_mut_ptr()) };
    return digest;
}

/// Built without OpenSSL, so there is nothing to hash with:
/// whichever solution asked for a hash skips right here
#[cfg(not(have_md5))]
pub fn md5(_input: &[u8]) -> Digest {
    crate::return_incomplete();
    return [0; 16];
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
