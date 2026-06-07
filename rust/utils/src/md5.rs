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

pub fn md5(input: &[u8]) -> [u8; 16] {
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
