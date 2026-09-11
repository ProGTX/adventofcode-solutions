#[repr(C)]
struct MD5_CTX {
    state: [u32; 4],
    count: [u32; 2],
    buffer: [u8; 64],
}

unsafe extern "C" {
    fn MD5Init(ctx: *mut MD5_CTX);
    fn MD5Update(ctx: *mut MD5_CTX, input: *const u8, input_len: u32);
    fn MD5Final(digest: *mut u8, ctx: *mut MD5_CTX);
}

pub type Digest = [u8; 16];

pub fn md5(input: &[u8]) -> Digest {
    unsafe {
        let mut ctx = MD5_CTX {
            state: [0; 4],
            count: [0; 2],
            buffer: [0; 64],
        };
        MD5Init(&mut ctx);
        MD5Update(&mut ctx, input.as_ptr(), input.len() as u32);
        let mut digest = [0u8; 16];
        MD5Final(digest.as_mut_ptr(), &mut ctx);
        digest
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
