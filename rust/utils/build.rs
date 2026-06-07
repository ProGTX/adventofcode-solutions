fn main() {
    let md5_lib = std::env::var("MD5_LIB").unwrap();
    let md5_lib_path = std::path::Path::new(&md5_lib);
    println!("cargo:rustc-link-search=native={}", md5_lib_path.parent().unwrap().display());
    println!("cargo:rustc-link-lib=static=md5");
    println!("cargo:rerun-if-changed={md5_lib}");
}
