fn main() {
    println!("cargo::rerun-if-env-changed=OPENSSL_CRYPTO_LIB");
    println!("cargo::rustc-check-cfg=cfg(have_md5)");

    // OpenSSL is optional, and CMake only passes this on once it has found it.
    // Without it `aoc::md5` skips the solution instead of hashing,
    // which is also what a plain `cargo build` outside CMake gets
    let Ok(crypto_lib) = std::env::var("OPENSSL_CRYPTO_LIB") else {
        return;
    };
    let crypto_lib_path = std::path::Path::new(&crypto_lib);
    if let Some(directory) = crypto_lib_path.parent() {
        println!("cargo::rustc-link-search=native={}", directory.display());
    }
    println!("cargo::rustc-link-lib=crypto");
    println!("cargo::rustc-cfg=have_md5");
    println!("cargo::rerun-if-changed={crypto_lib}");
}
