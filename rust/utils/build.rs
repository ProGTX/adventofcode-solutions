fn main() {
    println!("cargo::rerun-if-env-changed=AOC_MD5_LIB");

    // CMake builds the MD5 library and passes its path.
    // Outside CMake there is nothing to point at,
    // and a build that needs it fails at link time
    let Ok(md5_lib) = std::env::var("AOC_MD5_LIB") else {
        return;
    };
    let md5_lib_path = std::path::Path::new(&md5_lib);
    let Some(directory) = md5_lib_path.parent() else {
        return;
    };
    // libaoc_md5.a and aoc_md5.lib are both linked as aoc_md5
    let name = md5_lib_path
        .file_stem()
        .and_then(|stem| stem.to_str())
        .map_or("aoc_md5", |stem| stem.strip_prefix("lib").unwrap_or(stem));

    println!("cargo::rustc-link-search=native={}", directory.display());
    println!("cargo::rustc-link-lib=static={name}");
    // The library is C++, so its standard library comes along
    if cfg!(target_env = "msvc") {
        // The MSVC runtime comes in through the static library itself
    } else {
        println!("cargo::rustc-link-lib=stdc++");
    }
    println!("cargo::rerun-if-changed={md5_lib}");
}
